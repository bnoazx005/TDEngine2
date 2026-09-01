#include "../../include/ecs/CSpriteRendererSystem.h"
#include "../../include/ecs/CWorld.h"
#include "../../include/graphics/CQuadSprite.h"
#include "../../include/editor/CPerfProfiler.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/ecs/CSystemManager.h"
#include "../../include/ecs/CBoundsUpdatingSystem.h"
#include "../../include/ecs/components/CBoundsComponent.h"
#include "../../include/core/memory/CBaseAllocator.h"
#include "../../include/graphics/CRenderQueue.h"
#include "../../include/graphics/IGraphicsObjectManager.h"
#include "../../include/graphics/IVertexDeclaration.h"
#include "../../include/graphics/CGraphicsLayersInfo.h"
#include "../../include/graphics/ITexture.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/core/memory/CLinearAllocator.h"
#include "../../include/graphics/IRenderer.h"
#include "../../include/graphics/ICamera.h"
#include "../../include/graphics/CFramePacketsStorage.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/graphics/CBaseMaterial.h"
#include "../../include/core/memory/IAllocator.h"
#include "../../include/core/IJobManager.h"
#include "../../include/utils/CFileLogger.h"


namespace TDEngine2
{
	CSpriteRendererSystem::TBatchEntry::TBatchEntry(TPtr<IAllocator> pAllocator):
		mInstancesData(CSTLAllocatorWrapper<CSpriteRendererSystem::TSpriteInstanceData>(pAllocator))
	{
	}


	CSpriteRendererSystem::CSpriteRendererSystem() :
		CBaseSystem(), 
		mSpriteVertexBufferHandle(TBufferHandleId::Invalid), 
		mSpriteIndexBufferHandle(TBufferHandleId::Invalid),
		mSpriteFaces {0, 1, 2, 2, 1, 3}, 
		mpGraphicsLayers(nullptr),
		mpRenderer(nullptr),
		mpGraphicsObjectManager(nullptr)
	{
	}

	E_RESULT_CODE CSpriteRendererSystem::Init(TPtr<IAllocator> allocator, IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager)
	{
		TDE2_PROFILER_SCOPE("CSpriteRendererSystem::Init");

		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pGraphicsObjectManager || !pRenderer)
		{
			return RC_INVALID_ARGS;
		}

		mpTempAllocator   = allocator;
		mpRenderer        = pRenderer;
		mpResourceManager = mpRenderer->GetResourceManager();

		mpGraphicsObjectManager = pGraphicsObjectManager;

		TSpriteVertex vertices[] =
		{
			{ TVector4(-0.5f, 0.5f, 0.0f, 1.0f), TVector2(0.0f, 0.0f) },
			{ TVector4(0.5f, 0.5f, 0.0f, 1.0f), TVector2(1.0f, 0.0f) },
			{ TVector4(-0.5f, -0.5f, 0.0f, 1.0f), TVector2(0.0f, 1.0f) },
			{ TVector4(0.5f, -0.5f, 0.0f, 1.0f), TVector2(1.0f, 1.0f) }
		};

		mSpriteVertexBufferHandle = pGraphicsObjectManager->CreateBuffer({ 
				E_BUFFER_USAGE_TYPE::STATIC, 
				E_BUFFER_TYPE::STRUCTURED,
				sizeof(TSpriteVertex) * 4, 
				&vertices[0], 
				sizeof(TSpriteVertex) * 4, 
				false,
				sizeof(TSpriteVertex),
				E_STRUCTURED_BUFFER_TYPE::DEFAULT
			}).Get();

		mSpriteIndexBufferHandle = pGraphicsObjectManager->CreateBuffer({ 
				E_BUFFER_USAGE_TYPE::STATIC,
				E_BUFFER_TYPE::STRUCTURED,
				sizeof(U32) * 6,
				mSpriteFaces,
				sizeof(U32) * 6,
				false,
				sizeof(U32),
				E_STRUCTURED_BUFFER_TYPE::DEFAULT
			}).Get();

		E_RESULT_CODE result = RC_OK;
		
		mpGraphicsLayers = TPtr<IGraphicsLayersInfo>(CreateGraphicsLayersInfo(result));
		if (result != RC_OK)
		{
			return result;
		}

		mpGraphicsLayers->AddLayer(4.0f, "Layer1");

		mIsInitialized = true;

		return RC_OK;
	}

	void CSpriteRendererSystem::InjectBindings(IWorld* pWorld)
	{
		TDE2_PROFILER_SCOPE("CSpriteRendererSystem::InjectBindings");

		mTransforms.clear();		
		mSprites.clear();
		mSpritesBounds.clear();
		mBatches.clear();

		TEntitiesArray entities = pWorld->FindEntitiesWithComponents<CTransform, CQuadSprite, CBoundsComponent>();
		for (TEntityId currEntityId : entities)
		{
			CEntity* pCurrEntity = pWorld->FindEntity(currEntityId);
			if (!pCurrEntity)
			{
				continue;
			}

			mTransforms.push_back(pCurrEntity->GetComponent<CTransform>());
			mSprites.push_back(pCurrEntity->GetComponent<CQuadSprite>());
			mSpritesBounds.push_back(pCurrEntity->GetComponent<CBoundsComponent>());

			TDE2_ASSERT(mTransforms.back());
			TDE2_ASSERT(mSprites.back());
			TDE2_ASSERT(mSpritesBounds.back());
		}
	}


	static inline U32 ComputeSpriteCommandKey(TResourceId materialId, U16 graphicsLayerId)
	{
		return static_cast<U32>(materialId) << 16 | graphicsLayerId;
	}


	void CSpriteRendererSystem::Update(IWorld* pWorld, F32 dt)
	{
		TDE2_PROFILER_SCOPE("CSpriteRendererSystem::Update");

		mBatches.clear();

		if (mpTempAllocator)
		{
			mpTempAllocator->Clear();
		}

		TDrawIndexedInstancedCommand* pCurrCommand = nullptr;

		CTransform* pCurrTransform = nullptr;
		CQuadSprite* pCurrSprite = nullptr;
		CBoundsComponent* pCurrBounds = nullptr;

		struct TLocalBatchEntry
		{
			std::vector<TSpriteInstanceData> mInstancesData{};
			TResourceId                      mMaterialHandle = TResourceId::Invalid;
		};

		typedef std::unordered_map<U32, TLocalBatchEntry> TLocalTBatchesBuffer;

		mpJobManager->SubmitJob(&mMainSystemJobCounter, [this, pWorld](auto)
			{
				TJobCounter counter{};

				constexpr U32 GROUP_SIZE = 2;

				Vector<TLocalTBatchesBuffer> perGroupBatches(static_cast<USIZE>(std::ceil(mSprites.size() / GROUP_SIZE)));

				mpJobManager->SubmitMultipleJobs(&counter, static_cast<U32>(mSprites.size()), GROUP_SIZE, [this, pWorld, &perGroupBatches](const TJobArgs& args)
					{
						TDE2_PROFILER_SCOPE("CSpriteRendererSystem::ProcessSprite");

						ICamera* pCameraComponent = GetCurrentActiveCamera(pWorld);
						if (!pCameraComponent)
						{
							LOG_WARNING("[CSpriteRendererSystem] An entity with Camera component attached to that wasn't found, frustum culling is disabled");
						}

						CTransform* pCurrTransform = mTransforms[args.mJobIndex];
						CQuadSprite* pCurrSprite = mSprites[args.mJobIndex];
						CBoundsComponent* pCurrBounds = mSpritesBounds[args.mJobIndex];

						TQuadSpriteComponentData& spriteData = pCurrSprite->GetData();

						if (pCameraComponent)
						{
							TPtr<IFrustum> pCameraFrustum = pCameraComponent->GetFrustum();
							if (pCameraFrustum && !pCameraFrustum->TestAABB(pCurrBounds->GetBounds()))
							{
								return;
							}
						}

						const TResourceId currMaterialHandle = mpResourceManager->Load<IMaterial>(spriteData.mMaterialName);
						const U32 groupKey = ComputeSpriteCommandKey(currMaterialHandle, mpGraphicsLayers->GetLayerIndex(pCurrTransform->GetPosition().z));

						TLocalTBatchesBuffer& localBatches = perGroupBatches[args.mGroupIndex];
						auto it = localBatches.find(groupKey);
						if (it == localBatches.end())
						{
							it = localBatches.emplace(groupKey, TLocalBatchEntry{}).first;
						}

						TLocalBatchEntry& currBatchEntry = it->second;

						currBatchEntry.mMaterialHandle = currMaterialHandle;
						currBatchEntry.mInstancesData.push_back({ Transpose(pCurrTransform->GetLocalToWorldTransform()), spriteData.mColor });
					});

				mpJobManager->WaitForJobCounter(counter);

				{
					TDE2_PROFILER_SCOPE("CSpriteRendererSystem::GatcherBatches");

					for (const TLocalTBatchesBuffer& currGroupBatch : perGroupBatches)
					{
						for (auto& currBatchEntry : currGroupBatch)
						{
							auto destBatchIt = mBatches.find(currBatchEntry.first);
							if (destBatchIt == mBatches.end())
							{
								destBatchIt = mBatches.emplace(currBatchEntry.first, mpTempAllocator).first;
							}

							destBatchIt->second.mMaterialHandle = currBatchEntry.second.mMaterialHandle;
							destBatchIt->second.mInstancesData.insert(destBatchIt->second.mInstancesData.end(),
								std::make_move_iterator(currBatchEntry.second.mInstancesData.begin()),
								std::make_move_iterator(currBatchEntry.second.mInstancesData.end()));
						}
					}
				}
			});
	}

	E_RESULT_CODE CSpriteRendererSystem::FillFramePacket(TFramePacket& framePacket)
	{
		TDE2_PROFILER_SCOPE("CSpriteRendererSystem::FillFramePacket");

		U32 currInstancesBufferIndex = 0;

		TPtr<CRenderQueue> pRenderQueue = framePacket.mpRenderQueues[static_cast<U32>(E_RENDER_QUEUE_GROUP::RQG_SPRITES)];

		TFramePacket::TSpritesFrameData& spritesRenderData = framePacket.mSpritesBatchesData;
		
		for (auto& currBatchPairEntry : mBatches)
		{
			TDE2_PROFILER_SCOPE("CSpriteRendererSystem::processBatch");

			TBufferHandleId currInstancingBufferHandle = TBufferHandleId::Invalid;

			if (spritesRenderData.mSpritesPerInstanceDataHandles.size() <= currInstancesBufferIndex)
			{
				auto createInstancesBufferResult = mpGraphicsObjectManager->CreateBuffer(
					{
						E_BUFFER_USAGE_TYPE::DYNAMIC,
						E_BUFFER_TYPE::STRUCTURED,
						SPRITE_INSTANCE_DATA_BUFFER_SIZE,
						nullptr,
						SPRITE_INSTANCE_DATA_BUFFER_SIZE,
						false,
						sizeof(TSpriteInstanceData),
						E_STRUCTURED_BUFFER_TYPE::DEFAULT
					});

				if (createInstancesBufferResult.HasError())
				{
					LOG_ERROR(Wrench::StringUtils::Format("[CSpriteRendererSystem] Failed to create instances buffer: {0}", createInstancesBufferResult.GetError()));
					return createInstancesBufferResult.GetError();
				}

				currInstancingBufferHandle = spritesRenderData.mSpritesPerInstanceDataHandles.emplace_back(createInstancesBufferResult.Get());
			}

			currInstancingBufferHandle = spritesRenderData.mSpritesPerInstanceDataHandles[currInstancesBufferIndex++];

			TPtr<IBuffer> pCurrBatchInstancesBuffer = mpGraphicsObjectManager->GetBufferPtr(currInstancingBufferHandle);

			TBatchEntry& currBatchEntry = currBatchPairEntry.second;

			const U32 instancesCount = static_cast<U32>(currBatchEntry.mInstancesData.size());
			const U32 currBatchSize = instancesCount * sizeof(TSpriteInstanceData);

			if (!instancesCount)
			{
				continue;
			}

			if (currBatchSize <= SPRITE_INSTANCE_DATA_BUFFER_SIZE)
			{
				pCurrBatchInstancesBuffer->Map(E_BUFFER_MAP_TYPE::BMT_WRITE_DISCARD);
				pCurrBatchInstancesBuffer->Write(currBatchEntry.mInstancesData.data(), currBatchSize);
				pCurrBatchInstancesBuffer->Unmap();
			}

			TDrawIndexedInstancedCommand* pCurrCommand = pRenderQueue->SubmitDrawCommand<TDrawIndexedInstancedCommand>(currBatchPairEntry.first); /// \note currBatchPairEntry.first is a group key that was computed before

			pCurrCommand->mVertexBufferHandle     = mSpriteVertexBufferHandle;
			pCurrCommand->mIndexBufferHandle      = mSpriteIndexBufferHandle;
			pCurrCommand->mPrimitiveType          = E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_LIST;
			pCurrCommand->mIndicesPerInstance     = 6;
			pCurrCommand->mBaseVertexIndex        = 0;
			pCurrCommand->mStartIndex             = 0;
			pCurrCommand->mStartInstance          = 0;
			pCurrCommand->mNumOfInstances         = instancesCount;/// assign number of sprites in a batch
			pCurrCommand->mInstancingBufferHandle = currInstancingBufferHandle; /// assign accumulated data of a batch
			pCurrCommand->mMaterialHandle         = currBatchEntry.mMaterialHandle;

			TPtr<IMaterial> pMaterial = mpResourceManager->GetResource<IMaterial>(currBatchEntry.mMaterialHandle);
			ITexture* pMainTexture = pMaterial->GetTextureResource(Wrench::StringUtils::GetEmptyStr());

			auto&& uvRect = pMainTexture ? pMainTexture->GetNormalizedTextureRect() : TRectF32{ 0.0f, 0.0f, 1.0f, 1.0f };

			pCurrCommand->mObjectData.mModelMatrix          = IdentityMatrix4;
			pCurrCommand->mObjectData.mTextureTransformDesc = { uvRect.x, uvRect.y, uvRect.width, uvRect.height };

			currBatchEntry.mInstancesData.clear();
		}

		return RC_OK;
	}


	TDE2_API ISystem* CreateSpriteRendererSystem(TPtr<IAllocator> allocator, IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CSpriteRendererSystem, result, allocator, pRenderer, pGraphicsObjectManager);
	}


	struct TSpritesRendererSystemAutoInitializer : TSystemAutoInitializer
	{
		virtual ~TSpritesRendererSystemAutoInitializer() = default;

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
			pSystemInstance = CreateSpriteRendererSystem(
				TPtr<IAllocator>(CreateLinearAllocator(5 * CSpriteRendererSystem::SPRITE_INSTANCE_DATA_BUFFER_SIZE, result)),
				PolymorphicCast<IRenderer*>(subsystemsProviderCallback(E_ENGINE_SUBSYSTEM_TYPE::EST_RENDERER)),
				PolymorphicCast<IGraphicsContext*>(subsystemsProviderCallback(E_ENGINE_SUBSYSTEM_TYPE::EST_GRAPHICS_CONTEXT))->GetGraphicsObjectManager(), result);

			return pSystemInstance;
		}

		ISystem* pSystemInstance = nullptr;
	};


	TDE2_REGISTER_SYSTEM(TSpritesRendererSystemAutoInitializer);
}