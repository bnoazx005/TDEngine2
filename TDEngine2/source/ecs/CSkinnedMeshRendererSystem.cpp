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
#include <cassert>
#include <cmath>


namespace TDEngine2
{
	CSkinnedMeshRendererSystem::CSkinnedMeshRendererSystem() :
		CBaseSystem()
	{
	}


	static TResourceId DepthOnlyMaterialHandle = TResourceId::Invalid;
	static IVertexDeclaration* pDepthOnlyVertDecl = nullptr;


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

		mpRenderer              = pRenderer;
		mpGraphicsObjectManager = pGraphicsObjectManager;
		mpResourceManager       = pRenderer->GetResourceManager();

		DepthOnlyMaterialHandle = mpResourceManager->Create<IMaterial>("SkinnedDepthOnly.material", TMaterialParameters
			{
				"Shaders/Default/SkinnedDepthOnly.shader", false,
				TDepthStencilStateDesc { true, true, E_COMPARISON_FUNC::LESS_EQUAL},
				TRasterizerStateDesc { E_CULL_MODE::NONE, false, false, 0.0f, 0.0f, false }
			});

		if (auto newVertDeclResult = mpGraphicsObjectManager->CreateVertexDeclaration())
		{
			pDepthOnlyVertDecl = newVertDeclResult.Get();
			pDepthOnlyVertDecl->AddElement({ FT_FLOAT4, 0, VEST_POSITION });
		}

		mIsInitialized = true;

		return RC_OK;
	}

	void CSkinnedMeshRendererSystem::InjectBindings(IWorld* pWorld)
	{
		TEntitiesArray entities = pWorld->FindEntitiesWithComponents<CTransform, CSkinnedMeshContainer, CBoundsComponent>();

		mProcessingEntities.clear();

		CEntity* pCurrEntity = nullptr;

		for (auto iter = entities.begin(); iter != entities.end(); ++iter)
		{
			pCurrEntity = pWorld->FindEntity(*iter);

			if (!pCurrEntity)
			{
				continue;
			}

			mProcessingEntities.push_back({ pCurrEntity->GetComponent<CTransform>(), pCurrEntity->GetComponent<CSkinnedMeshContainer>(), pCurrEntity->GetComponent<CBoundsComponent>() });
		}

		mpCameraComponent = GetCurrentActiveCamera(pWorld);;
	}

	void CSkinnedMeshRendererSystem::Update(IWorld* pWorld, F32 dt)
	{
		TDE2_PROFILER_SCOPE("CSkinnedMeshRendererSystem::Update");

		if (!mpCameraComponent)
		{
			LOG_WARNING("[CSkinnedMeshRendererSystem] An entity with Camera component attached to that wasn't found");
			return;
		}

		if (mProcessingEntities.empty())
		{
			return;
		}

		mpJobManager->SubmitJob(&mMainSystemJobCounter, [this](auto)
			{
				// \note first pass (construct an array of materials)
				// \note Materials: | {opaque_material_group1}, ..., {opaque_material_groupN} | {transp_material_group1}, ..., {transp_material_groupM} |
				const USIZE opaqueMaterialsCount = _collectUsedMaterials(mProcessingEntities, mpResourceManager.Get(), mCurrMaterialsArray);

				// \note construct commands for opaque geometry
				for (USIZE i = 0; i < opaqueMaterialsCount; ++i)
				{
					_prepareLocalRenderCommands(mProcessingEntities, mCurrMaterialsArray[i], mpCameraComponent, mVisibleOpaqueMeshes);
				}

				// \note construct commands for transparent geometry
				for (USIZE i = opaqueMaterialsCount; i < mCurrMaterialsArray.size(); ++i)
				{
					_prepareLocalRenderCommands(mProcessingEntities, mCurrMaterialsArray[i], mpCameraComponent, mVisibleTransparentMeshes);
				}
			});
	}


	static void AddRenderCommandInternal(TPtr<IResourceManager> pResourceManager, TPtr<CRenderQueue> pMainRenderQueue, TPtr<CRenderQueue> pDepthOnlyRenderQueue, const CSkinnedMeshRendererSystem::TMeshDrawEntry& meshEntry)
	{
		TDE2_PROFILER_SCOPE("CSkinnedMeshRendererSystem::AddRenderCommandInternal");

		TDrawIndexedCommand* pCommand = pMainRenderQueue->SubmitDrawCommand<TDrawIndexedCommand>(
			static_cast<U32>(meshEntry.mGeometrySubGroupTag) + (static_cast<U32>(meshEntry.mMaterialHandle) << 16) | static_cast<U16>(fabs(meshEntry.mDistanceToCamera)));

		TPtr<ISkinnedMesh> pSharedMeshResource = pResourceManager->GetResource<ISkinnedMesh>(meshEntry.mMeshHandle);
		if (!pSharedMeshResource)
		{
			return;
		}

		pCommand->mVertexBufferHandle = meshEntry.mSharedPositionOnlyVertexBufferHandle;
		pCommand->mIndexBufferHandle = pSharedMeshResource->GetSharedIndexBuffer();

		for (U32 i = 1; i < static_cast<U32>(E_VERTEX_STREAM_TYPE::COUNT); ++i)
		{
			pCommand->mAdditionalVertexBuffers[i - 1] = pSharedMeshResource->GetVertexBufferForStream(static_cast<E_VERTEX_STREAM_TYPE>(i));
		}

		pCommand->mMaterialHandle                = meshEntry.mMaterialHandle;
		pCommand->mMaterialInstanceId            = meshEntry.mMaterialInstanceId;
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


	E_RESULT_CODE CSkinnedMeshRendererSystem::FillFramePacket(TFramePacket& framePacket)
	{
		TDE2_PROFILER_SCOPE("CSkinnedMeshRendererSystem::FillFramePacket");

		TPtr<CRenderQueue> pOpaqueRenderGroup    = framePacket.mpRenderQueues[static_cast<U32>(E_RENDER_QUEUE_GROUP::RQG_OPAQUE_GEOMETRY)];
		TPtr<CRenderQueue> pDepthOnlyRenderGroup = framePacket.mpRenderQueues[static_cast<U32>(E_RENDER_QUEUE_GROUP::RQG_DEPTH_PREPASS)];

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

	USIZE CSkinnedMeshRendererSystem::_collectUsedMaterials(const TSystemContext& entities, IResourceManager* pResourceManager, TMaterialsArray& usedMaterials)
	{
		usedMaterials.clear();

		TJobCounter counter{};
		std::mutex mutex;

		std::atomic_size_t opaqueMaterialsCount = 0;

		mpJobManager->SubmitMultipleJobs(&counter, static_cast<U32>(entities.size()), 1, [&](const TJobArgs& args)
			{
				CSkinnedMeshContainer* pCurrSkinnedMeshContainer = nullptr;

				TPtr<IMaterial> pCurrMaterial;

				for (auto& iter : entities)
				{
					pCurrSkinnedMeshContainer = std::get<CSkinnedMeshContainer*>(iter);

					pCurrMaterial = mpResourceManager->GetResource<IMaterial>(mpResourceManager->Load<IMaterial>(pCurrSkinnedMeshContainer->GetMaterialName()));

					// \note skip duplicates
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
						std::lock_guard<std::mutex> lock(mutex);
						// \note skip duplicates
						if (std::find(usedMaterials.cbegin(), usedMaterials.cend(), pCurrMaterial) != usedMaterials.cend())
						{
							return;
						}

						usedMaterials.insert(isTransparent ? usedMaterials.end() : usedMaterials.begin(), pCurrMaterial);
					}
				}
			});

		mpJobManager->WaitForJobCounter(counter);

		return std::min<USIZE>(opaqueMaterialsCount, usedMaterials.size());
	}


	static E_RESULT_CODE ShowSkeletonDebugHierarchy(IGraphicsObjectManager* pGraphicsObjectsManager, IResourceManager* pResourceManager, IRenderer* pRenderer, 
													const std::vector<TMatrix4>& currPose, TResourceId skeletonId)
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

		TPtr<ISkeleton> pSkeleton = pResourceManager->GetResource<ISkeleton>(skeletonId);
		if (!pSkeleton)
		{
			return RC_FAIL;
		}

		pSkeleton->ForEachJoint([&currPose, pDebugUtility, pSkeleton](TJoint* pJoint)
		{
			const TVector4 first = Transpose(currPose[pJoint->mIndex]) * TVector4(ZeroVector3, 1.0f);

			if (pJoint->mParentIndex >= 0)
			{
				if (TJoint* pParentJoint = pSkeleton->GetJoint(pJoint->mParentIndex))
				{
					pDebugUtility->DrawLine(first, Transpose(currPose[pParentJoint->mIndex]) * TVector4(ZeroVector3, 1.0f), TColorUtils::mYellow);
				}
			}

			pDebugUtility->DrawCross(first, 2.0f, TColorUtils::mGreen);
		});

		return RC_OK;
	}


	void CSkinnedMeshRendererSystem::_prepareLocalRenderCommands(const TSystemContext& entities, TPtr<IMaterial> pCurrMaterial, const ICamera* pCamera, Vector<TMeshDrawEntry>& visibleMeshes)
	{
		auto iter = entities.begin();

		auto&& pCastedMaterial = DynamicPtrCast<CBaseMaterial>(pCurrMaterial);
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
			auto pBounds               = std::get<CBoundsComponent*>(*iter);

			if (!pCamera->GetFrustum()->TestAABB(pBounds->GetBounds()))
			{
				++iter;
				continue;
			}

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
				pSkinnedMeshContainer->SetSystemBuffersHandle(static_cast<U32>(mMeshBuffersMap.size()));

#if TDE2_EDITORS_ENABLED
				for (auto&& currSubmeshId : pSharedMeshResource->GetSubmeshesIdentifiers())
				{
					pSkinnedMeshContainer->AddSubmeshIdentifier(currSubmeshId);
				}
#endif
			}

			if (pSkinnedMeshContainer->IsDirty())
			{
				pSkinnedMeshContainer->SetSubMeshRenderInfo(pSharedMeshResource->GetSubmeshInfo(pSkinnedMeshContainer->GetSubMeshId()));
				pSkinnedMeshContainer->SetDirty(false);
			}

			const TSubMeshRenderInfo& subMeshInfo = pSkinnedMeshContainer->GetSubMeshInfo();

			auto& currAnimationPose = pSkinnedMeshContainer->GetCurrentAnimationPose();
			U32 jointsCount = static_cast<U32>(currAnimationPose.size());

			auto&& skeletonName = pSkinnedMeshContainer->GetSkeletonName();
			if (skeletonName.empty() || !jointsCount)
			{
				++iter;
				continue;
			}

			const TResourceId skeletonResourceId = mpResourceManager->Load<ISkeleton>(pSkinnedMeshContainer->GetSkeletonName());

			auto pSkeletonResource = mpResourceManager->GetResource<IResource>(sharedMeshId);
			if (!pSkeletonResource || (pSkeletonResource && (E_RESOURCE_STATE_TYPE::RST_LOADED != pSkeletonResource->GetState())))
			{
				++iter;
				continue;
			}

			/// \note Get or create a new material's instance
			TMaterialInstanceId materialInstance = pSkinnedMeshContainer->GetMaterialInstanceHandle();
			if (TMaterialInstanceId::Invalid == materialInstance)
			{
				materialInstance = pCastedMaterial->CreateInstance()->GetInstanceId();
				pSkinnedMeshContainer->SetMaterialInstanceHandle(materialInstance);
			}

			if (pSkinnedMeshContainer->ShouldShowDebugSkeleton())
			{
				TDE2_ASSERT(RC_OK == ShowSkeletonDebugHierarchy(mpGraphicsObjectManager, mpResourceManager.Get(), mpRenderer, currAnimationPose, skeletonResourceId));
			}

			pCastedMaterial->SetVariableForInstance(materialInstance, CSkinnedMeshContainer::mJointsArrayUniformVariableId, &currAnimationPose.front(), static_cast<U32>(sizeof(TMatrix4) * currAnimationPose.size()));
			pCastedMaterial->SetVariableForInstance(materialInstance, CSkinnedMeshContainer::mJointsCountUniformVariableId, &jointsCount, sizeof(U32));

			auto&& objectTransformMatrix = pTransform->GetLocalToWorldTransform();

			TMeshDrawEntry meshDrawCommandEntry{};
			meshDrawCommandEntry.mDistanceToCamera                     = ((viewMatrix * objectTransformMatrix) * TVector4(0.0f, 0.0f, 1.0f, 1.0f)).z;
			meshDrawCommandEntry.mGeometrySubGroupTag                  = pCastedMaterial->GetGeometrySubGroupTag();
			meshDrawCommandEntry.mSharedPositionOnlyVertexBufferHandle = pSharedMeshResource->GetVertexBufferForStream(E_VERTEX_STREAM_TYPE::POSITIONS);
			meshDrawCommandEntry.mSharedIndexBufferHandle              = pSharedMeshResource->GetSharedIndexBuffer();
			meshDrawCommandEntry.mMeshHandle                           = sharedMeshId;
			meshDrawCommandEntry.mMaterialHandle                       = currMaterialId;
			meshDrawCommandEntry.mMaterialInstanceId                   = materialInstance;
			meshDrawCommandEntry.mModelMat                             = Transpose(objectTransformMatrix);
			meshDrawCommandEntry.mInvModelMat                          = Transpose(Inverse(objectTransformMatrix));
			meshDrawCommandEntry.mStartIndex                           = subMeshInfo.mStartIndex;
			meshDrawCommandEntry.mIndicesCount                         = subMeshInfo.mIndicesCount;
			meshDrawCommandEntry.mVertexFormatFlags                    = pSharedMeshResource->GetVertexFormatFlags();

			visibleMeshes.emplace_back(meshDrawCommandEntry);

			++iter;
		}
	}


	TDE2_API ISystem* CreateSkinnedMeshRendererSystem(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CSkinnedMeshRendererSystem, result, pRenderer, pGraphicsObjectManager);
	}


	struct TSkinnedMeshRendererSystemAutoInitializer : TSystemAutoInitializer
	{
		virtual ~TSkinnedMeshRendererSystemAutoInitializer() = default;

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
			pSystemInstance = CreateSkinnedMeshRendererSystem(
				PolymorphicCast<IRenderer*>(subsystemsProviderCallback(E_ENGINE_SUBSYSTEM_TYPE::EST_RENDERER)),
				PolymorphicCast<IGraphicsContext*>(subsystemsProviderCallback(E_ENGINE_SUBSYSTEM_TYPE::EST_GRAPHICS_CONTEXT))->GetGraphicsObjectManager(), result);

			return pSystemInstance;
		}

		ISystem* pSystemInstance = nullptr;
	};


	TDE2_REGISTER_SYSTEM(TSkinnedMeshRendererSystemAutoInitializer);
}