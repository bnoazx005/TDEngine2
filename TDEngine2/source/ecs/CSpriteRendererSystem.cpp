#include "../../include/ecs/CSpriteRendererSystem.h"
#include "../../include/ecs/CWorld.h"
#include "../../include/graphics/CQuadSprite.h"
#include "../../include/editor/CPerfProfiler.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/ecs/components/CBoundsComponent.h"
#include "../../include/graphics/CRenderQueue.h"
#include "../../include/graphics/IGraphicsObjectManager.h"
#include "../../include/graphics/IVertexDeclaration.h"
#include "../../include/graphics/CGraphicsLayersInfo.h"
#include "../../include/graphics/ITexture.h"
#include "../../include/core/IGraphicsContext.h"
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

		mpTempAllocator = allocator;

		mpRenderer = pRenderer;
		mpFramePacketsStorage = mpRenderer->GetFramePacketsStorage().Get();

		mpResourceManager = mpRenderer->GetResourceManager();

		mpGraphicsObjectManager = pGraphicsObjectManager;

		/// pre allocated temporary buffer for batching sprites instancing data
		_initializeBatchVertexBuffers(mpGraphicsObjectManager, PreCreatedNumOfVertexBuffers);

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
		
		mpGraphicsLayers = CreateGraphicsLayersInfo(result);
		mpGraphicsLayers->AddLayer(4.0f, "Layer1");

		if (result != RC_OK)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CSpriteRendererSystem::_onFreeInternal()
	{
		E_RESULT_CODE result = RC_OK;

		if ((result = mpGraphicsLayers->Free()) != RC_OK)
		{
			return result;
		}

		for (auto iter = mBatches.begin(); iter != mBatches.end(); ++iter)
		{
			TBatchEntry& currBatchEntry = (*iter).second;

			delete currBatchEntry.mpInstancesData;
		}

		return RC_OK;
	}

	void CSpriteRendererSystem::InjectBindings(IWorld* pWorld)
	{
		TDE2_PROFILER_SCOPE("CSpriteRendererSystem::InjectBindings");

		std::vector<TEntityId> entities = pWorld->FindEntitiesWithComponents<CTransform, CQuadSprite>();

		mTransforms.clear();		
		mSprites.clear();
		mSpritesBounds.clear();
		mBatches.clear();

		CEntity* pCurrEntity = nullptr;

		for (auto iter = entities.begin(); iter != entities.end(); ++iter)
		{
			pCurrEntity = pWorld->FindEntity(*iter);
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

	void CSpriteRendererSystem::Update(IWorld* pWorld, F32 dt)
	{
		TDE2_PROFILER_SCOPE("CSpriteRendererSystem::Update");

		TDrawIndexedInstancedCommand* pCurrCommand = nullptr;

		CTransform* pCurrTransform = nullptr;
		CQuadSprite* pCurrSprite = nullptr;
		CBoundsComponent* pCurrBounds = nullptr;

		U32 groupKey = 0x0;

		/// allocate memory for vertex buffers that will store instances data if it's not allocated yet
		if (mSpritesPerInstanceDataHandles.empty())
		{
			_initializeBatchVertexBuffers(mpGraphicsObjectManager, PreCreatedNumOfVertexBuffers);
		}

		mpJobManager->SubmitJob(&mMainSystemJobCounter, [this, pWorld](auto)
			{
				TJobCounter counter{};
				mpJobManager->SubmitMultipleJobs(&counter, static_cast<U32>(mSprites.size()), 2, [this, pWorld](const TJobArgs& args)
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

						bool isVisible = true;
						if (pCameraComponent)
						{
							IFrustum* pCameraFrustum = pCameraComponent->GetFrustum();
							if (pCameraFrustum && !pCameraFrustum->TestAABB(pCurrBounds->GetBounds()))
							{
								return;
							}
						}

						const TResourceId currMaterialHandle = mpResourceManager->Load<IMaterial>(pCurrSprite->GetMaterialName());

						U32 groupKey = _computeSpriteCommandKey(currMaterialHandle, mpGraphicsLayers->GetLayerIndex(pCurrTransform->GetPosition().z));

						TBatchEntry& currBatchEntry = mBatches[groupKey];

						currBatchEntry.mMaterialHandle = currMaterialHandle;

						if (!currBatchEntry.mpInstancesData)
						{
							currBatchEntry.mpInstancesData = new CDynamicArray<TSpriteInstanceData>(*mpTempAllocator.Get(), 100);
						}

						currBatchEntry.mpInstancesData->PushBack({ Transpose(pCurrTransform->GetLocalToWorldTransform()), pCurrSprite->GetColor() });
					});

				mpJobManager->WaitForJobCounter(counter);

				mpJobManager->SubmitJob(&counter, [this](auto)
					{
						U32 currInstancesBufferIndex = 0;

						TPtr<IBuffer> pCurrBatchInstancesBuffer = nullptr;

						TPtr<CRenderQueue> pRenderQueue = mpFramePacketsStorage->GetCurrentFrameForGameLogic().mpRenderQueues[static_cast<U32>(E_RENDER_QUEUE_GROUP::RQG_SPRITES)];

						for (auto iter = mBatches.begin(); iter != mBatches.end(); ++iter)
						{
							TDE2_PROFILER_SCOPE("CSpriteRendererSystem::processBatch");

							const TBufferHandleId currInstancingBufferHandle = mSpritesPerInstanceDataHandles[currInstancesBufferIndex++];
							pCurrBatchInstancesBuffer = mpGraphicsObjectManager->GetBufferPtr(currInstancingBufferHandle);

							TBatchEntry& currBatchEntry = (*iter).second;

							U32 instancesCount = static_cast<U32>(currBatchEntry.mpInstancesData->GetSize());
							U32 currBatchSize = instancesCount * sizeof(TSpriteInstanceData);

							if (!instancesCount)
							{
								continue;
							}

							if (currBatchSize <= SPRITE_INSTANCE_DATA_BUFFER_SIZE)
							{
								pCurrBatchInstancesBuffer->Map(E_BUFFER_MAP_TYPE::BMT_WRITE_DISCARD);
								pCurrBatchInstancesBuffer->Write(&(*currBatchEntry.mpInstancesData)[0], currBatchSize);
								pCurrBatchInstancesBuffer->Unmap();
							}

							TPtr<IMaterial> pMaterial = mpResourceManager->GetResource<IMaterial>(currBatchEntry.mMaterialHandle);
							ITexture* pMainTexture = pMaterial->GetTextureResource(Wrench::StringUtils::GetEmptyStr());

							TDrawIndexedInstancedCommand* pCurrCommand = pRenderQueue->SubmitDrawCommand<TDrawIndexedInstancedCommand>((*iter).first); /// \note (*iter).first is a group key that was computed before

							pCurrCommand->mVertexBufferHandle = mSpriteVertexBufferHandle;
							pCurrCommand->mIndexBufferHandle = mSpriteIndexBufferHandle;
							pCurrCommand->mPrimitiveType = E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_LIST;
							pCurrCommand->mIndicesPerInstance = 6;
							pCurrCommand->mBaseVertexIndex = 0;
							pCurrCommand->mStartIndex = 0;
							pCurrCommand->mStartInstance = 0;
							pCurrCommand->mNumOfInstances = instancesCount;/// assign number of sprites in a batch
							pCurrCommand->mInstancingBufferHandle = currInstancingBufferHandle; /// assign accumulated data of a batch
							pCurrCommand->mMaterialHandle = currBatchEntry.mMaterialHandle;

							auto&& uvRect = pMainTexture ? pMainTexture->GetNormalizedTextureRect() : TRectF32{ 0.0f, 0.0f, 1.0f, 1.0f };

							pCurrCommand->mObjectData.mModelMatrix = IdentityMatrix4;
							pCurrCommand->mObjectData.mTextureTransformDesc = { uvRect.x, uvRect.y, uvRect.width, uvRect.height };

							currBatchEntry.mpInstancesData->Clear();
						}
					});
			});
	}

	U32 CSpriteRendererSystem::_computeSpriteCommandKey(TResourceId materialId, U16 graphicsLayerId)
	{
		return static_cast<U32>(materialId) << 16 | graphicsLayerId;
	}

	void CSpriteRendererSystem::_initializeBatchVertexBuffers(IGraphicsObjectManager* pGraphicsObjectManager, U32 numOfBuffers)
	{
		TDE2_PROFILER_SCOPE("CSpriteRendererSystem::_initializeBatchVertexBuffers");

		for (U32 i = 0; i < numOfBuffers; ++i)
		{
			mSpritesPerInstanceDataHandles.push_back(
				pGraphicsObjectManager->CreateBuffer({
						E_BUFFER_USAGE_TYPE::DYNAMIC,
						E_BUFFER_TYPE::STRUCTURED, 
						SPRITE_INSTANCE_DATA_BUFFER_SIZE,
						nullptr,
						SPRITE_INSTANCE_DATA_BUFFER_SIZE,
						false,
						sizeof(TSpriteInstanceData),
						E_STRUCTURED_BUFFER_TYPE::DEFAULT 
					}).Get());
		}
	}


	TDE2_API ISystem* CreateSpriteRendererSystem(TPtr<IAllocator> allocator, IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CSpriteRendererSystem, result, allocator, pRenderer, pGraphicsObjectManager);
	}
}