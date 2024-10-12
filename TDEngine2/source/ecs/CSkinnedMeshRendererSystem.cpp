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
#include "../../include/ecs/components/CBoundsComponent.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/core/IJobManager.h"
#include "../../include/graphics/CPerspectiveCamera.h"
#include "../../include/graphics/COrthoCamera.h"
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
		mpFramePacketsStorage   = pRenderer->GetFramePacketsStorage().Get();
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
		std::vector<TEntityId> entities = pWorld->FindEntitiesWithComponents<CTransform, CSkinnedMeshContainer, CBoundsComponent>();

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
				_collectUsedMaterials(mProcessingEntities, mpResourceManager.Get(), mCurrMaterialsArray);

				const U32 opaqueMaterialsCount = static_cast<U32>(std::min<USIZE>(mCurrMaterialsArray.size(), std::distance(mCurrMaterialsArray.cbegin(), std::find_if(mCurrMaterialsArray.cbegin(), mCurrMaterialsArray.cend(),
					[](auto&& pCurrMaterial) { return pCurrMaterial->IsTransparent(); }))));

				CRenderQueue* pOpaqueRenderGroup = mpFramePacketsStorage->GetCurrentFrameForGameLogic().mpRenderQueues[static_cast<U32>(E_RENDER_QUEUE_GROUP::RQG_OPAQUE_GEOMETRY)].Get();
				CRenderQueue* pTransparentRenderGroup = mpFramePacketsStorage->GetCurrentFrameForGameLogic().mpRenderQueues[static_cast<U32>(E_RENDER_QUEUE_GROUP::RQG_TRANSPARENT_GEOMETRY)].Get();
				CRenderQueue* pDepthOnlyRenderGroup = mpFramePacketsStorage->GetCurrentFrameForGameLogic().mpRenderQueues[static_cast<U32>(E_RENDER_QUEUE_GROUP::RQG_DEPTH_PREPASS)].Get();

				// \note construct commands for opaque geometry
				for (U32 i = 0; i < opaqueMaterialsCount; ++i)
				{
					_populateCommandsBuffer(mProcessingEntities, pOpaqueRenderGroup, pDepthOnlyRenderGroup, mCurrMaterialsArray[i], mpCameraComponent);
				}

				// \note construct commands for transparent geometry
				for (U32 i = opaqueMaterialsCount; i < mCurrMaterialsArray.size(); ++i)
				{
					_populateCommandsBuffer(mProcessingEntities, pTransparentRenderGroup, nullptr, mCurrMaterialsArray[i], mpCameraComponent);
				}
			});
	}

	void CSkinnedMeshRendererSystem::_collectUsedMaterials(const TEntitiesArray& entities, IResourceManager* pResourceManager, TMaterialsArray& usedMaterials)
	{
		usedMaterials.clear();

		TJobCounter counter{};

		std::mutex mutex;

		mpJobManager->SubmitMultipleJobs(&counter, static_cast<U32>(entities.size()), 1, [&](const TJobArgs& args)
			{
				ISkinnedMeshContainer* pCurrSkinnedMeshContainer = nullptr;

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

					{
						std::lock_guard<std::mutex> lock(mutex);
						// \note skip duplicates
						if (std::find(usedMaterials.cbegin(), usedMaterials.cend(), pCurrMaterial) != usedMaterials.cend())
						{
							return;
						}

						usedMaterials.insert(pCurrMaterial->IsTransparent() ? usedMaterials.end() : usedMaterials.begin(), pCurrMaterial);
					}
				}
			});

		mpJobManager->WaitForJobCounter(counter);
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


	void CSkinnedMeshRendererSystem::_populateCommandsBuffer(const TEntitiesArray& entities, CRenderQueue*& pRenderGroup, CRenderQueue* pDepthOnlyRenderGroup,
															TPtr<IMaterial> pCurrMaterial, const ICamera* pCamera)
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

			auto& meshBuffersEntry = mMeshBuffersMap[pSkinnedMeshContainer->GetSystemBuffersHandle()];

			auto&& objectTransformMatrix = pTransform->GetLocalToWorldTransform();

			F32 distanceToCamera = ((viewMatrix * objectTransformMatrix) * TVector4(0.0f, 0.0f, 1.0f, 1.0f)).z;

			// create a command for the renderer
			auto pCommand = pRenderGroup->SubmitDrawCommand<TDrawIndexedCommand>(static_cast<U32>(pCastedMaterial->GetGeometrySubGroupTag()) + 
																				 _computeMeshCommandHash(currMaterialId, distanceToCamera));
			
			TDE2_ASSERT(pCommand);

			pCommand->mVertexBufferHandle         = pSharedMeshResource->GetSharedVertexBuffer();
			pCommand->mIndexBufferHandle          = pSharedMeshResource->GetSharedIndexBuffer();
			pCommand->mMaterialHandle             = currMaterialId;
			pCommand->mMaterialInstanceId         = materialInstance;
			pCommand->mpVertexDeclaration         = meshBuffersEntry.mpVertexDecl; // \todo replace with access to a vertex declarations pool
			pCommand->mNumOfIndices               = subMeshInfo.mIndicesCount;
			pCommand->mStartIndex                 = subMeshInfo.mStartIndex;
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

	U32 CSkinnedMeshRendererSystem::_computeMeshCommandHash(TResourceId materialId, F32 distanceToCamera)
	{
		return (static_cast<U32>(materialId) << 16) | static_cast<U16>(fabs(distanceToCamera));
	}


	TDE2_API ISystem* CreateSkinnedMeshRendererSystem(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CSkinnedMeshRendererSystem, result, pRenderer, pGraphicsObjectManager);
	}
}