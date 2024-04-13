#include "../../include/ecs/CStaticMeshRendererSystem.h"
#include "../../include/graphics/IGraphicsObjectManager.h"
#include "../../include/graphics/IRenderer.h"
#include "../../include/graphics/CStaticMeshContainer.h"
#include "../../include/graphics/CRenderQueue.h"
#include "../../include/graphics/CBaseMaterial.h"
#include "../../include/graphics/IVertexDeclaration.h"
#include "../../include/graphics/CStaticMesh.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/ecs/CWorld.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/graphics/CPerspectiveCamera.h"
#include "../../include/graphics/COrthoCamera.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/editor/CPerfProfiler.h"
#include <algorithm>
#include <cmath>


namespace TDEngine2
{
	CStaticMeshRendererSystem::CStaticMeshRendererSystem() :
		CBaseSystem()
	{
	}


	static TResourceId DepthOnlyMaterialHandle = TResourceId::Invalid;
	static IVertexDeclaration* pDepthOnlyVertDecl = nullptr;


	E_RESULT_CODE CStaticMeshRendererSystem::Init(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pGraphicsObjectManager || !pRenderer)
		{
			return RC_INVALID_ARGS;
		}

		mpOpaqueRenderGroup      = pRenderer->GetRenderQueue(E_RENDER_QUEUE_GROUP::RQG_OPAQUE_GEOMETRY);
		mpTransparentRenderGroup = pRenderer->GetRenderQueue(E_RENDER_QUEUE_GROUP::RQG_TRANSPARENT_GEOMETRY);
		mpDepthOnlyRenderGroup   = pRenderer->GetRenderQueue(E_RENDER_QUEUE_GROUP::RQG_DEPTH_PREPASS);
		
		mpGraphicsObjectManager = pGraphicsObjectManager;

		mpResourceManager = pRenderer->GetResourceManager();

		DepthOnlyMaterialHandle = mpResourceManager->Create<IMaterial>("DepthOnly.material", TMaterialParameters
			{
				"Shaders/Default/DepthOnly.shader", false,
				TDepthStencilStateDesc { true, true, E_COMPARISON_FUNC::LESS_EQUAL},
				TRasterizerStateDesc { E_CULL_MODE::NONE, false, false, 0.1f, 1.0f, false } 
			});
		
		if (auto newVertDeclResult = mpGraphicsObjectManager->CreateVertexDeclaration())
		{
			pDepthOnlyVertDecl = newVertDeclResult.Get();
			pDepthOnlyVertDecl->AddElement({ FT_FLOAT4, 0, VEST_POSITION });
		}

		mIsInitialized = true;

		return RC_OK;
	}

	void CStaticMeshRendererSystem::InjectBindings(IWorld* pWorld)
	{
		std::vector<TEntityId> entities = pWorld->FindEntitiesWithComponents<CTransform, CStaticMeshContainer>();

		mProcessingEntities.clear();

		CEntity* pCurrEntity = nullptr;

		for (auto iter = entities.begin(); iter != entities.end(); ++iter)
		{
			pCurrEntity = pWorld->FindEntity(*iter);

			if (!pCurrEntity)
			{
				continue;
			}

			mProcessingEntities.push_back({ pCurrEntity->GetComponent<CTransform>(), pCurrEntity->GetComponent<CStaticMeshContainer>() });
		}
	}

	void CStaticMeshRendererSystem::Update(IWorld* pWorld, F32 dt)
	{
		TDE2_PROFILER_SCOPE("CStaticMeshRendererSystem::Update");

		ICamera* pCameraComponent = GetCurrentActiveCamera(pWorld);
		TDE2_ASSERT(pCameraComponent);
		if (!pCameraComponent)
		{
			LOG_WARNING("[CStaticMeshRendererSystem] An entity with Camera component attached to that wasn't found");
			return;
		}

		// \note first pass (construct an array of materials)
		// \note Materials: | {opaque_material_group1}, ..., {opaque_material_groupN} | {transp_material_group1}, ..., {transp_material_groupM} |
		_collectUsedMaterials(mProcessingEntities, mpResourceManager.Get(), mCurrMaterialsArray);

		auto firstTransparentMatIter = std::find_if(mCurrMaterialsArray.cbegin(), mCurrMaterialsArray.cend(), [](auto&& pCurrMaterial)
		{
			return pCurrMaterial->IsTransparent();
		});
		
		// \note construct commands for opaque geometry
		std::for_each(mCurrMaterialsArray.cbegin(), firstTransparentMatIter, [this, &pCameraComponent](auto&& pCurrMaterial)
		{
			_populateCommandsBuffer(mProcessingEntities, mpOpaqueRenderGroup, mpDepthOnlyRenderGroup, pCurrMaterial, pCameraComponent);
		});

		// \note construct commands for transparent geometry
		std::for_each(firstTransparentMatIter, mCurrMaterialsArray.cend(), [this, &pCameraComponent](auto&& pCurrMaterial)
		{
			_populateCommandsBuffer(mProcessingEntities, mpTransparentRenderGroup, nullptr, pCurrMaterial, pCameraComponent);
		});
	}

	void CStaticMeshRendererSystem::_collectUsedMaterials(const TEntitiesArray& entities, IResourceManager* pResourceManager, TMaterialsArray& usedMaterials)
	{
		usedMaterials.clear();

		IStaticMeshContainer* pCurrStaticMeshContainer = nullptr;

		TPtr<IMaterial> pCurrMaterial;

		for (auto& iter : entities)
		{
			pCurrStaticMeshContainer = std::get<CStaticMeshContainer*>(iter);

			auto&& currMaterialId = pCurrStaticMeshContainer->GetMaterialName();
			if (currMaterialId.empty())
			{
				continue;
			}

			pCurrMaterial = mpResourceManager->GetResource<IMaterial>(mpResourceManager->Load<IMaterial>(currMaterialId));
			if (!pCurrMaterial)
			{
				continue;
			}

			// \note skip duplicates
			if (std::find(usedMaterials.cbegin(), usedMaterials.cend(), pCurrMaterial) != usedMaterials.cend())
			{
				continue;
			}

			usedMaterials.push_back(pCurrMaterial);
		}

		// sort all materials
		std::sort(usedMaterials.begin(), usedMaterials.end(), CBaseMaterial::AlphaBasedMaterialComparator);
	}

	void CStaticMeshRendererSystem::_populateCommandsBuffer(const TEntitiesArray& entities, CRenderQueue*& pRenderGroup, CRenderQueue* pDepthOnlyRenderGroup,
															TPtr<IMaterial> pCurrMaterial, const ICamera* pCamera)
	{
		auto iter = entities.begin();

		auto&& pCastedMaterial = DynamicPtrCast<CBaseMaterial>(pCurrMaterial);
		const std::string& currMaterialName = pCastedMaterial->GetName();

		TResourceId currMaterialId = pCastedMaterial->GetId();

		auto&& viewMatrix = pCamera->GetViewMatrix();

		// \note iterate over all entities with pCurrMaterial attached as main material
		while ((iter = std::find_if(iter, entities.end(), [currMaterialId, &currMaterialName](auto&& entity)
		{
			return std::get<CStaticMeshContainer*>(entity)->GetMaterialName() == currMaterialName;
		})) != entities.end())
		{
			auto pStaticMeshContainer = std::get<CStaticMeshContainer*>(*iter);
			auto pTransform           = std::get<CTransform*>(*iter);

			const TResourceId sharedMeshId = mpResourceManager->Load<IStaticMesh>(pStaticMeshContainer->GetMeshName());

			auto pSharedMeshResource = mpResourceManager->GetResource<IStaticMesh>(sharedMeshId);
			if (!pSharedMeshResource || (pSharedMeshResource && (E_RESOURCE_STATE_TYPE::RST_LOADED != mpResourceManager->GetResource<IResource>(sharedMeshId)->GetState())))
			{
				++iter;
				continue;
			}

			// \note we need to create vertex and index buffers for the object
			if (pStaticMeshContainer->GetSystemBuffersHandle() == static_cast<U32>(-1))
			{
				pStaticMeshContainer->SetSystemBuffersHandle(static_cast<U32>(mMeshBuffersMap.size()));

				auto pVertexDecl = mpGraphicsObjectManager->CreateVertexDeclaration().Get();

				mMeshBuffersMap.push_back({ pSharedMeshResource->GetSharedVertexBuffer(), pSharedMeshResource->GetSharedIndexBuffer(), pVertexDecl });

				// \note form the vertex declaration for the mesh
				pVertexDecl->AddElement({ TDEngine2::FT_FLOAT4, 0, TDEngine2::VEST_POSITION });
				pVertexDecl->AddElement({ TDEngine2::FT_FLOAT4, 0, TDEngine2::VEST_COLOR });

				if (pSharedMeshResource->HasTexCoords0())
				{
					pVertexDecl->AddElement({ TDEngine2::FT_FLOAT4, 0, TDEngine2::VEST_TEXCOORDS });
				}

				if (pSharedMeshResource->HasNormals())
				{
					pVertexDecl->AddElement({ TDEngine2::FT_FLOAT4, 0, TDEngine2::VEST_NORMAL });
				}

				if (pSharedMeshResource->HasTangents())
				{
					pVertexDecl->AddElement({ TDEngine2::FT_FLOAT4, 0, TDEngine2::VEST_TANGENT });
				}

#if TDE2_EDITORS_ENABLED
				for (auto&& currSubmeshId : pSharedMeshResource->GetSubmeshesIdentifiers())
				{
					pStaticMeshContainer->AddSubmeshIdentifier(currSubmeshId);
				}
#endif
			}

			if (pStaticMeshContainer->IsDirty())
			{
				pStaticMeshContainer->SetSubMeshRenderInfo(pSharedMeshResource->GetSubmeshInfo(pStaticMeshContainer->GetSubMeshId()));
				pStaticMeshContainer->SetDirty(false);
			}

			const TSubMeshRenderInfo& subMeshInfo = pStaticMeshContainer->GetSubMeshInfo();

			auto meshBuffersEntry = mMeshBuffersMap[pStaticMeshContainer->GetSystemBuffersHandle()];

			auto&& objectTransformMatrix = pTransform->GetLocalToWorldTransform();

			F32 distanceToCamera = ((viewMatrix * objectTransformMatrix) * TVector4(0.0f, 0.0f, 1.0f, 1.0f)).z;

			// create a command for the renderer
			auto pCommand = pRenderGroup->SubmitDrawCommand<TDrawIndexedCommand>(static_cast<U32>(pCastedMaterial->GetGeometrySubGroupTag()) + 
																				 _computeMeshCommandHash(currMaterialId, distanceToCamera));

			pCommand->mVertexBufferHandle         = pSharedMeshResource->GetSharedVertexBuffer();
			pCommand->mIndexBufferHandle          = pSharedMeshResource->GetSharedIndexBuffer();
			pCommand->mMaterialHandle             = mpResourceManager->Load<IMaterial>(pStaticMeshContainer->GetMaterialName());
			pCommand->mpVertexDeclaration         = meshBuffersEntry.mpVertexDecl; // \todo replace with access to a vertex declarations pool
			pCommand->mStartIndex                 = subMeshInfo.mStartIndex;
			pCommand->mNumOfIndices               = subMeshInfo.mIndicesCount;
			pCommand->mPrimitiveType              = E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_LIST;
			pCommand->mObjectData.mModelMatrix    = Transpose(objectTransformMatrix);
			pCommand->mObjectData.mInvModelMatrix = Transpose(Inverse(objectTransformMatrix));

			if (pDepthOnlyRenderGroup && E_GEOMETRY_SUBGROUP_TAGS::SKYBOX != pCastedMaterial->GetGeometrySubGroupTag())
			{
				auto pDepthOnlyCommand = pDepthOnlyRenderGroup->SubmitDrawCommand<TDrawIndexedCommand>(static_cast<U32>(fabs(distanceToCamera)));

				pDepthOnlyCommand->mVertexBufferHandle = pSharedMeshResource->GetPositionOnlyVertexBuffer();
				pDepthOnlyCommand->mIndexBufferHandle = pCommand->mIndexBufferHandle;
				pDepthOnlyCommand->mMaterialHandle = DepthOnlyMaterialHandle;
				pDepthOnlyCommand->mpVertexDeclaration = pDepthOnlyVertDecl;
				pDepthOnlyCommand->mStartIndex = pCommand->mStartIndex;
				pDepthOnlyCommand->mNumOfIndices = pCommand->mNumOfIndices;
				pDepthOnlyCommand->mPrimitiveType = pCommand->mPrimitiveType;
				pDepthOnlyCommand->mObjectData.mModelMatrix = pCommand->mObjectData.mModelMatrix;
				pDepthOnlyCommand->mObjectData.mInvModelMatrix = pCommand->mObjectData.mInvModelMatrix;
			}

			++iter;
		}
	}

	U32 CStaticMeshRendererSystem::_computeMeshCommandHash(TResourceId materialId, F32 distanceToCamera)
	{
		return (static_cast<U32>(materialId) << 16) | static_cast<U16>(fabs(distanceToCamera));
	}


	TDE2_API ISystem* CreateStaticMeshRendererSystem(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CStaticMeshRendererSystem, result, pRenderer, pGraphicsObjectManager);
	}
}