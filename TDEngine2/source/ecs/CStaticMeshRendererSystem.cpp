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
#include "../../include/ecs/CSystemManager.h"
#include "../../include/ecs/CBoundsUpdatingSystem.h"
#include "../../include/ecs/components/CBoundsComponent.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/core/IJobManager.h"
#include "../../include/graphics/CCamera.h"
#include "../../include/graphics/CFramePacketsStorage.h"
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

		mpGraphicsObjectManager = pGraphicsObjectManager;
		mpResourceManager       = pRenderer->GetResourceManager();

		DepthOnlyMaterialHandle = mpResourceManager->Create<IMaterial>("DepthOnly.material", TMaterialParameters
			{
				"Shaders/Default/DepthOnly.shader", false,
				TDepthStencilStateDesc { true, true, E_COMPARISON_FUNC::LESS_EQUAL},
				TRasterizerStateDesc { E_CULL_MODE::NONE, false, false, 0.1f, 1.0f, false } 
			});
		
		mIsInitialized = true;

		return RC_OK;
	}

	void CStaticMeshRendererSystem::InjectBindings(IWorld* pWorld)
	{
		TEntitiesArray entities = pWorld->FindEntitiesWithComponents<CTransform, CStaticMeshContainer, CBoundsComponent>();

		mProcessingEntities.clear();

		CEntity* pCurrEntity = nullptr;

		for (auto iter = entities.begin(); iter != entities.end(); ++iter)
		{
			pCurrEntity = pWorld->FindEntity(*iter);

			if (!pCurrEntity)
			{
				continue;
			}

			mProcessingEntities.push_back({ pCurrEntity->GetComponent<CTransform>(), pCurrEntity->GetComponent<CStaticMeshContainer>(), pCurrEntity->GetComponent<CBoundsComponent>() });
		}

		mpCurrActiveCamera = GetCurrentActiveCamera(pWorld);
	}

	void CStaticMeshRendererSystem::Update(IWorld* pWorld, F32 dt)
	{
		TDE2_PROFILER_SCOPE("CStaticMeshRendererSystem::Update");

		if (!mpCurrActiveCamera)
		{
			LOG_WARNING("[CStaticMeshRendererSystem] An entity with Camera component attached to that wasn't found");
			return;
		}

		// \note first pass (construct an array of materials)
		// \note Materials: | {opaque_material_group1}, ..., {opaque_material_groupN} | {transp_material_group1}, ..., {transp_material_groupM} |
		const USIZE opaqueMaterialsCount = _collectUsedMaterials(mProcessingEntities, mpResourceManager.Get(), mCurrMaterialsArray);

		for (USIZE i = 0; i < opaqueMaterialsCount; ++i)
		{
			_prepareLocalRenderCommands(mProcessingEntities, mCurrMaterialsArray[i], mpCurrActiveCamera, mVisibleOpaqueMeshes);
		}

		for (USIZE i = opaqueMaterialsCount; i < mCurrMaterialsArray.size(); ++i)
		{
			_prepareLocalRenderCommands(mProcessingEntities, mCurrMaterialsArray[i], mpCurrActiveCamera, mVisibleTransparentMeshes);
		}
	}


	static void AddRenderCommandInternal(TPtr<IResourceManager> pResourceManager, TPtr<CRenderQueue> pMainRenderQueue, TPtr<CRenderQueue> pDepthOnlyRenderQueue, const CStaticMeshRendererSystem::TMeshDrawEntry& meshEntry)
	{
		TDE2_PROFILER_SCOPE("CStaticMeshRendererSystem::AddRenderCommandInternal");

		TDrawIndexedCommand* pCommand = pMainRenderQueue->SubmitDrawCommand<TDrawIndexedCommand>(
			static_cast<U32>(meshEntry.mGeometrySubGroupTag) + (static_cast<U32>(meshEntry.mMaterialHandle) << 16) | static_cast<U16>(fabs(meshEntry.mDistanceToCamera)));

		TPtr<IStaticMesh> pSharedMeshResource = pResourceManager->GetResource<IStaticMesh>(meshEntry.mMeshHandle);
		if (!pSharedMeshResource)
		{
			return;
		}

		pCommand->mVertexBufferHandle = meshEntry.mSharedPositionOnlyVertexBufferHandle;
		pCommand->mIndexBufferHandle = pSharedMeshResource->GetSharedIndexBuffer();

		for (U32 i = 1; i < static_cast<U32>(E_VERTEX_STREAM_TYPE::SKINNING); ++i)
		{
			pCommand->mAdditionalVertexBuffers[i - 1] = pSharedMeshResource->GetVertexBufferForStream(static_cast<E_VERTEX_STREAM_TYPE>(i));
		}

		pCommand->mMaterialHandle                = meshEntry.mMaterialHandle;
		pCommand->mStartIndex                    = meshEntry.mStartIndex;
		pCommand->mNumOfIndices                  = meshEntry.mIndicesCount;
		pCommand->mPrimitiveType                 = E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_LIST;
		pCommand->mObjectData.mModelMatrix       = meshEntry.mModelMat;
		pCommand->mObjectData.mInvModelMatrix    = meshEntry.mInvModelMat;
		pCommand->mObjectData.mStartIndexOffset  = meshEntry.mStartIndex;
		pCommand->mObjectData.mVertexFormatFlags = meshEntry.mVertexFormatFlags;

		if (pDepthOnlyRenderQueue && E_GEOMETRY_SUBGROUP_TAGS::SKYBOX != meshEntry.mGeometrySubGroupTag)
		{
			auto pDepthOnlyCommand = pDepthOnlyRenderQueue->SubmitDrawCommand<TDrawIndexedCommand>(static_cast<U32>(fabs(meshEntry.mDistanceToCamera)));

			pDepthOnlyCommand->mVertexBufferHandle           = pSharedMeshResource->GetVertexBufferForStream(E_VERTEX_STREAM_TYPE::POSITIONS);
			pDepthOnlyCommand->mIndexBufferHandle            = pCommand->mIndexBufferHandle;
			pDepthOnlyCommand->mMaterialHandle               = DepthOnlyMaterialHandle;
			pDepthOnlyCommand->mStartIndex                   = pCommand->mStartIndex;
			pDepthOnlyCommand->mNumOfIndices                 = pCommand->mNumOfIndices;
			pDepthOnlyCommand->mPrimitiveType                = pCommand->mPrimitiveType;
			pDepthOnlyCommand->mObjectData.mModelMatrix      = pCommand->mObjectData.mModelMatrix;
			pDepthOnlyCommand->mObjectData.mInvModelMatrix   = pCommand->mObjectData.mInvModelMatrix;
			pDepthOnlyCommand->mObjectData.mStartIndexOffset = pCommand->mStartIndex;
			pCommand->mObjectData.mVertexFormatFlags         = pCommand->mObjectData.mVertexFormatFlags;
		}
	}


	E_RESULT_CODE CStaticMeshRendererSystem::FillFramePacket(TFramePacket& framePacket)
	{
		TDE2_PROFILER_SCOPE("CStaticMeshRendererSystem::FillFramePacket");

		TPtr<CRenderQueue> pOpaqueRenderGroup      = framePacket.mpRenderQueues[static_cast<U32>(E_RENDER_QUEUE_GROUP::RQG_OPAQUE_GEOMETRY)];
		TPtr<CRenderQueue> pDepthOnlyRenderGroup   = framePacket.mpRenderQueues[static_cast<U32>(E_RENDER_QUEUE_GROUP::RQG_DEPTH_PREPASS)];

		for (const TMeshDrawEntry& currMeshEntry : mVisibleOpaqueMeshes)
		{
			AddRenderCommandInternal(mpResourceManager, pOpaqueRenderGroup, pDepthOnlyRenderGroup, currMeshEntry);
		}

		TPtr<CRenderQueue> pTransparentRenderGroup = framePacket.mpRenderQueues[static_cast<U32>(E_RENDER_QUEUE_GROUP::RQG_TRANSPARENT_GEOMETRY)];

		for (const TMeshDrawEntry& currMeshEntry : mVisibleTransparentMeshes)
		{
			AddRenderCommandInternal(mpResourceManager, pTransparentRenderGroup, nullptr, currMeshEntry);
		}

		mVisibleOpaqueMeshes.clear();
		mVisibleTransparentMeshes.clear();

		return RC_OK;
	}

	USIZE CStaticMeshRendererSystem::_collectUsedMaterials(const TSystemContext& entities, IResourceManager* pResourceManager, TMaterialsArray& usedMaterials)
	{
		TDE2_PROFILER_SCOPE("CStaticMeshRendererSystem::CollectUsedMaterials");

		usedMaterials.clear();

		TJobCounter counter{};

		std::atomic_size_t opaqueMaterialsCount = 0;

		mpJobManager->SubmitMultipleJobs(&counter, static_cast<U32>(entities.size()), 1, [&](const TJobArgs& args)
			{
				TDE2_PROFILER_SCOPE("CStaticMeshRendererSystem::CollectUsedMaterials");

				CStaticMeshContainer* pCurrStaticMeshContainer = std::get<CStaticMeshContainer*>(entities[args.mJobIndex]);

				auto&& currMaterialId = pCurrStaticMeshContainer->GetMaterialName();
				if (currMaterialId.empty())
				{
					return;
				}

				TPtr<IMaterial> pCurrMaterial = mpResourceManager->GetResource<IMaterial>(mpResourceManager->Load<IMaterial>(currMaterialId));
				if (!pCurrMaterial)
				{
					return;
				}

				const bool isTransparent = pCurrMaterial->IsTransparent();
				if (!isTransparent)
				{
					++opaqueMaterialsCount;
				}

				{
					std::lock_guard<std::mutex> lock(mMaterialsMutex);
					// \note skip duplicates
					if (std::find(usedMaterials.cbegin(), usedMaterials.cend(), pCurrMaterial) != usedMaterials.cend())
					{
						return;
					}

					usedMaterials.insert(isTransparent ? usedMaterials.end() : usedMaterials.begin(), pCurrMaterial);
				}
			});

		mpJobManager->WaitForJobCounter(counter);

		return std::min<USIZE>(usedMaterials.size(), opaqueMaterialsCount);
	}

	void CStaticMeshRendererSystem::_prepareLocalRenderCommands(const TSystemContext& entities, TPtr<IMaterial> pCurrMaterial, const ICamera* pCamera, Vector<TMeshDrawEntry>& visibleMeshes)
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
			auto pTransform               = std::get<CTransform*>(*iter);
			auto pBounds              = std::get<CBoundsComponent*>(*iter);

			if (E_GEOMETRY_SUBGROUP_TAGS::SKYBOX != pCastedMaterial->GetGeometrySubGroupTag() && (pCamera->GetFrustum() && !pCamera->GetFrustum()->TestAABB(pBounds->GetBounds())))
			{
				++iter;
				continue;
			}

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

			auto&& objectTransformMatrix = pTransform->GetLocalToWorldTransform();

			TMeshDrawEntry meshDrawCommandEntry{};
			meshDrawCommandEntry.mDistanceToCamera                     = ((viewMatrix * objectTransformMatrix) * TVector4(0.0f, 0.0f, 1.0f, 1.0f)).z;
			meshDrawCommandEntry.mGeometrySubGroupTag                  = pCastedMaterial->GetGeometrySubGroupTag();
			meshDrawCommandEntry.mSharedPositionOnlyVertexBufferHandle = pSharedMeshResource->GetVertexBufferForStream(E_VERTEX_STREAM_TYPE::POSITIONS);
			meshDrawCommandEntry.mSharedIndexBufferHandle              = pSharedMeshResource->GetSharedIndexBuffer();
			meshDrawCommandEntry.mMeshHandle                           = sharedMeshId;
			meshDrawCommandEntry.mMaterialHandle                       = currMaterialId;
			meshDrawCommandEntry.mModelMat                             = Transpose(objectTransformMatrix);
			meshDrawCommandEntry.mInvModelMat                          = Transpose(Inverse(objectTransformMatrix));
			meshDrawCommandEntry.mStartIndex                           = subMeshInfo.mStartIndex;
			meshDrawCommandEntry.mIndicesCount                         = subMeshInfo.mIndicesCount;
			meshDrawCommandEntry.mVertexFormatFlags                    = pSharedMeshResource->GetVertexFormatFlags();

			visibleMeshes.emplace_back(meshDrawCommandEntry);

			++iter;
		}
	}


	TDE2_API ISystem* CreateStaticMeshRendererSystem(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CStaticMeshRendererSystem, result, pRenderer, pGraphicsObjectManager);
	}


	struct TStaticMeshRendererSystemAutoInitializer : TSystemAutoInitializer
	{
		virtual ~TStaticMeshRendererSystemAutoInitializer() = default;

		E_RESULT_CODE ManageDependencies(IWorld* pWorld) override
		{
			TSystemId boundsUpdatingSystemHandle = pWorld->FindSystem<CBoundsUpdatingSystem>();
			if (boundsUpdatingSystemHandle == TSystemId::Invalid)
			{
				return RC_FAIL;
			}

			return pSystemInstance ? pSystemInstance->AddDependency(pWorld->GetSystem(boundsUpdatingSystemHandle)) : RC_FAIL;
		}

		ISystem* GetSystem(IWorld* pWorld, const TRequestSubsystemCallback& subsystemsProviderCallback)
		{
			E_RESULT_CODE result = RC_OK;
			pSystemInstance = CreateStaticMeshRendererSystem(
				PolymorphicCast<IRenderer*>(subsystemsProviderCallback(E_ENGINE_SUBSYSTEM_TYPE::EST_RENDERER)),
				PolymorphicCast<IGraphicsContext*>(subsystemsProviderCallback(E_ENGINE_SUBSYSTEM_TYPE::EST_GRAPHICS_CONTEXT))->GetGraphicsObjectManager(), result);

			return pSystemInstance;
		}

		ISystem* pSystemInstance = nullptr;
	};


	TDE2_REGISTER_SYSTEM(TStaticMeshRendererSystemAutoInitializer);
}