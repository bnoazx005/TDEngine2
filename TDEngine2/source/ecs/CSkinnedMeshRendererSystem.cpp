#include "../../include/ecs/CSkinnedMeshRendererSystem.h"
#include "../../include/graphics/IGraphicsObjectManager.h"
#include "../../include/graphics/IRenderer.h"
#include "../../include/graphics/CSkinnedMeshContainer.h"
#include "../../include/graphics/CRenderQueue.h"
#include "../../include/graphics/CBaseMaterial.h"
#include "../../include/graphics/IVertexDeclaration.h"
#include "../../include/graphics/CSkinnedMesh.h"
#include "../../include/graphics/ISkeleton.h"
#include "../../include/graphics/IDebugUtility.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/graphics/CPerspectiveCamera.h"
#include "../../include/graphics/COrthoCamera.h"
#include "../../include/utils/CFileLogger.h"
#include <algorithm>
#include <cassert>
#include <cmath>


namespace TDEngine2
{
	static const std::string JointsPalleteShaderVariableId = "mJoints";
	static const std::string JointsCountShaderVariableId   = "mUsedJointsCount";


	CSkinnedMeshRendererSystem::CSkinnedMeshRendererSystem() :
		CBaseSystem()
	{
	}

	E_RESULT_CODE CSkinnedMeshRendererSystem::Init(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pGraphicsObjectManager || !pRenderer)
		{
			return RC_INVALID_ARGS;
		}

		mpRenderer = pRenderer;

		mpOpaqueRenderGroup      = pRenderer->GetRenderQueue(E_RENDER_QUEUE_GROUP::RQG_OPAQUE_GEOMETRY);
		mpTransparentRenderGroup = pRenderer->GetRenderQueue(E_RENDER_QUEUE_GROUP::RQG_TRANSPARENT_GEOMETRY);

		mpGraphicsObjectManager = pGraphicsObjectManager;

		mpResourceManager = pRenderer->GetResourceManager();

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CSkinnedMeshRendererSystem::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	void CSkinnedMeshRendererSystem::InjectBindings(IWorld* pWorld)
	{
		std::vector<TEntityId> entities = pWorld->FindEntitiesWithComponents<CTransform, CSkinnedMeshContainer>();

		mProcessingEntities.clear();

		CEntity* pCurrEntity = nullptr;

		for (auto iter = entities.begin(); iter != entities.end(); ++iter)
		{
			pCurrEntity = pWorld->FindEntity(*iter);

			if (!pCurrEntity)
			{
				continue;
			}

			mProcessingEntities.push_back({ pCurrEntity->GetComponent<CTransform>(), pCurrEntity->GetComponent<CSkinnedMeshContainer>() });
		}

		const auto& cameras = pWorld->FindEntitiesWithAny<CPerspectiveCamera, COrthoCamera>();
		mpCameraEntity = !cameras.empty() ? pWorld->FindEntity(cameras.front()) : nullptr;
	}

	void CSkinnedMeshRendererSystem::Update(IWorld* pWorld, F32 dt)
	{
		if (!mpCameraEntity)
		{
			LOG_WARNING("[CSkinnedMeshRendererSystem] An entity with Camera component attached to that wasn't found");
			return;
		}

		ICamera* pCameraComponent = GetValidPtrOrDefault<ICamera*>(mpCameraEntity->GetComponent<CPerspectiveCamera>(), mpCameraEntity->GetComponent<COrthoCamera>());
		TDE2_ASSERT(pCameraComponent);

		// \note first pass (construct an array of materials)
		// \note Materials: | {opaque_material_group1}, ..., {opaque_material_groupN} | {transp_material_group1}, ..., {transp_material_groupM} |
		_collectUsedMaterials(mProcessingEntities, mpResourceManager, mCurrMaterialsArray);

		auto firstTransparentMatIter = std::find_if(mCurrMaterialsArray.begin(), mCurrMaterialsArray.end(), [](IMaterial* pCurrMaterial)
		{
			return pCurrMaterial->IsTransparent();
		});

		// \note construct commands for opaque geometry
		std::for_each(mCurrMaterialsArray.begin(), firstTransparentMatIter, [this, pCameraComponent](IMaterial* pCurrMaterial)
		{
			_populateCommandsBuffer(mProcessingEntities, mpOpaqueRenderGroup, pCurrMaterial, pCameraComponent);
		});

		// \note construct commands for transparent geometry
		std::for_each(firstTransparentMatIter, mCurrMaterialsArray.end(), [this, pCameraComponent](IMaterial* pCurrMaterial)
		{
			_populateCommandsBuffer(mProcessingEntities, mpTransparentRenderGroup, pCurrMaterial, pCameraComponent);
		});
	}

	void CSkinnedMeshRendererSystem::_collectUsedMaterials(const TEntitiesArray& entities, IResourceManager* pResourceManager, TMaterialsArray& usedMaterials)
	{
		usedMaterials.clear();

		ISkinnedMeshContainer* pCurrSkinnedMeshContainer = nullptr;

		IMaterial* pCurrMaterial = nullptr;

		for (auto& iter : entities)
		{
			pCurrSkinnedMeshContainer = std::get<CSkinnedMeshContainer*>(iter);

			pCurrMaterial = mpResourceManager->GetResource<IMaterial>(mpResourceManager->Load<IMaterial>(pCurrSkinnedMeshContainer->GetMaterialName()));

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


	static E_RESULT_CODE ShowSkeletonDebugHierarchy(IGraphicsObjectManager* pGraphicsObjectsManager, IResourceManager* pResourceManager, IRenderer* pRenderer, TResourceId skeletonId)
	{
		if (!pGraphicsObjectsManager || !pResourceManager || TResourceId::Invalid == skeletonId)
		{
			return RC_INVALID_ARGS;
		}

		auto retrievePtrResult = pGraphicsObjectsManager->CreateDebugUtility(pResourceManager, pRenderer);
		if (retrievePtrResult.HasError())
		{
			return retrievePtrResult.GetError();
		}

		IDebugUtility* pDebugUtility = retrievePtrResult.Get();

		ISkeleton* pSkeleton = pResourceManager->GetResource<ISkeleton>(skeletonId);
		if (!pSkeleton)
		{
			return RC_FAIL;
		}

		pSkeleton->ForEachJoint([pDebugUtility, pSkeleton](TJoint* pJoint)
		{
			const TVector4 first = Inverse(pJoint->mInvBindTransform) * TVector4(ZeroVector3, 1.0f);

			if (pJoint->mParentIndex >= 0)
			{
				if (TJoint* pParentJoint = pSkeleton->GetJoint(pJoint->mParentIndex))
				{
					pDebugUtility->DrawLine(first, Inverse(pParentJoint->mInvBindTransform) * TVector4(ZeroVector3, 1.0f), TColorUtils::mYellow);
				}
			}

			pDebugUtility->DrawCross(first, 2.0f, TColorUtils::mGreen);
		});

		return RC_OK;
	}


	void CSkinnedMeshRendererSystem::_populateCommandsBuffer(const TEntitiesArray& entities, CRenderQueue*& pRenderGroup, IMaterial* pCurrMaterial,
															const ICamera* pCamera)
	{
		auto iter = entities.begin();

		auto&& pCastedMaterial = dynamic_cast<CBaseMaterial*>(pCurrMaterial);
		const std::string& currMaterialName = pCastedMaterial->GetName();
		TDE2_ASSERT(pCastedMaterial);

		TResourceId currMaterialId = pCastedMaterial->GetId();

		auto&& viewMatrix = pCamera->GetViewMatrix();

		// \note iterate over all entities with pCurrMaterial attached as main material
		while ((iter = std::find_if(iter, entities.end(), [pCastedMaterial, currMaterialId, &currMaterialName](auto&& entity)
		{
			return std::get<CSkinnedMeshContainer*>(entity)->GetMaterialName() == currMaterialName;
		})) != entities.end())
		{
			auto pSkinnedMeshContainer = std::get<CSkinnedMeshContainer*>(*iter);
			auto pTransform            = std::get<CTransform*>(*iter);

			const TResourceId sharedMeshId = mpResourceManager->Load<ISkinnedMesh>(pSkinnedMeshContainer->GetMeshName());

			auto pSharedMeshResource = mpResourceManager->GetResource<ISkinnedMesh>(sharedMeshId);
			if (!pSharedMeshResource || (pSharedMeshResource && (E_RESOURCE_STATE_TYPE::RST_LOADED != mpResourceManager->GetResource<IResource>(sharedMeshId)->GetState())))
			{
				++iter;
				continue;
			}
			
			// \note we need to create vertex and index buffers for the object
			if (pSkinnedMeshContainer->GetSystemBuffersHandle() == static_cast<U32>(-1))
			{
				pSkinnedMeshContainer->SetSystemBuffersHandle(mMeshBuffersMap.size());

				auto pVertexDecl = mpGraphicsObjectManager->CreateVertexDeclaration().Get();

				mMeshBuffersMap.push_back({ pSharedMeshResource->GetSharedVertexBuffer(), pSharedMeshResource->GetSharedIndexBuffer(), pVertexDecl });

				// \note form the vertex declaration for the mesh
				pVertexDecl->AddElement({ FT_FLOAT4, 0, VEST_POSITION });
				pVertexDecl->AddElement({ FT_FLOAT4, 0, VEST_COLOR });

				if (pSharedMeshResource->HasTexCoords0())
				{
					pVertexDecl->AddElement({ FT_FLOAT4, 0, VEST_TEXCOORDS });
				}

				if (pSharedMeshResource->HasNormals())
				{
					pVertexDecl->AddElement({ FT_FLOAT4, 0, VEST_NORMAL });
				}

				if (pSharedMeshResource->HasTangents())
				{
					pVertexDecl->AddElement({ FT_FLOAT4, 0, VEST_TANGENT });
				}

				if (pSharedMeshResource->HasJointWeights())
				{
					pVertexDecl->AddElement({ FT_FLOAT4, 0, VEST_JOINT_WEIGHTS });
				}

				if (pSharedMeshResource->HasJointIndices())
				{
					pVertexDecl->AddElement({ FT_UINT4, 0, VEST_JOINT_INDICES });
				}
			}

			auto& currAnimationPose = pSkinnedMeshContainer->GetCurrentAnimationPose();
			U32 jointsCount = static_cast<U32>(currAnimationPose.size());

			const TResourceId skeletonResourceId = mpResourceManager->Load<ISkeleton>(pSkinnedMeshContainer->GetSkeletonName());

			/// \note Get or create a new material's instance
			TMaterialInstanceId materialInstance = pSkinnedMeshContainer->GetMaterialInstanceHandle();
			if (TMaterialInstanceId::Invalid == materialInstance)
			{
				materialInstance = pCastedMaterial->CreateInstance()->GetInstanceId();
				pSkinnedMeshContainer->SetMaterialInstanceHandle(materialInstance);

				if (TResourceId::Invalid != skeletonResourceId)
				{
					ISkeleton* pSkeleton = mpResourceManager->GetResource<ISkeleton>(skeletonResourceId);

					pSkeleton->ForEachJoint([&currAnimationPose](TJoint* pJoint)
					{
						currAnimationPose.push_back(Inverse(pJoint->mInvBindTransform));
					});

					jointsCount = static_cast<U32>(currAnimationPose.size());
				}
			}

			if (pSkinnedMeshContainer->ShouldShowDebugSkeleton())
			{
				TDE2_ASSERT(RC_OK == ShowSkeletonDebugHierarchy(mpGraphicsObjectManager, mpResourceManager, mpRenderer, skeletonResourceId));
			}
			
			pCastedMaterial->SetVariableForInstance(materialInstance, JointsPalleteShaderVariableId, &currAnimationPose.front(), static_cast<U32>(sizeof(TMatrix4) * currAnimationPose.size()));
			pCastedMaterial->SetVariableForInstance(materialInstance, JointsCountShaderVariableId, &jointsCount, sizeof(U32));

			auto& meshBuffersEntry = mMeshBuffersMap[pSkinnedMeshContainer->GetSystemBuffersHandle()];

			auto&& objectTransformMatrix = pTransform->GetLocalToWorldTransform();

			F32 distanceToCamera = ((viewMatrix * objectTransformMatrix) * TVector4(0.0f, 0.0f, 1.0f, 1.0f)).z;

			// create a command for the renderer
			auto pCommand = pRenderGroup->SubmitDrawCommand<TDrawIndexedCommand>(static_cast<U32>(pCastedMaterial->GetGeometrySubGroupTag()) + 
																				 _computeMeshCommandHash(currMaterialId, distanceToCamera));
			
			TDE2_ASSERT(pCommand);

			pCommand->mpVertexBuffer              = pSharedMeshResource->GetSharedVertexBuffer();
			pCommand->mpIndexBuffer               = pSharedMeshResource->GetSharedIndexBuffer();
			pCommand->mMaterialHandle             = mpResourceManager->Load<IMaterial>(pSkinnedMeshContainer->GetMaterialName());
			pCommand->mMaterialInstanceId         = materialInstance;
			pCommand->mpVertexDeclaration         = meshBuffersEntry.mpVertexDecl; // \todo replace with access to a vertex declarations pool
			pCommand->mNumOfIndices               = pSharedMeshResource->GetIndices().size();
			pCommand->mPrimitiveType              = E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_LIST;
			pCommand->mObjectData.mModelMatrix    = Transpose(objectTransformMatrix);
			pCommand->mObjectData.mInvModelMatrix = Transpose(Inverse(objectTransformMatrix));

			++iter;
		}
	}

	U32 CSkinnedMeshRendererSystem::_computeMeshCommandHash(TResourceId materialId, F32 distanceToCamera)
	{
		return (static_cast<U32>(materialId) << 16) | static_cast<U16>(fabs(distanceToCamera));
	}


	TDE2_API ISystem* CreateSkinnedMeshRendererSystem(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CSkinnedMeshRendererSystem, result, pRenderer, pGraphicsObjectManager);
	}
}