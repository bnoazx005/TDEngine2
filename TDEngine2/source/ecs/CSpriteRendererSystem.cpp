#include "../../include/ecs/CSpriteRendererSystem.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/graphics/CQuadSprite.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/graphics/CRenderQueue.h"
#include "../../include/graphics/IGraphicsObjectManager.h"
#include "../../include/graphics/IVertexBuffer.h"
#include "../../include/graphics/IIndexBuffer.h"
#include "../../include/graphics/IVertexDeclaration.h"
#include "../../include/graphics/CGraphicsLayersInfo.h"
#include "../../include/graphics/ITexture.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/graphics/IRenderer.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/graphics/CBaseMaterial.h"
#include "../../include/core/memory/IAllocator.h"


namespace TDEngine2
{
	CSpriteRendererSystem::CSpriteRendererSystem() :
		CBaseSystem(), mpRenderQueue(nullptr), mpSpriteVertexBuffer(nullptr), mpSpriteIndexBuffer(nullptr),
		mpSpriteVertexDeclaration(nullptr), mSpriteFaces {0, 1, 2, 2, 1, 3}, mpGraphicsLayers(nullptr)
	{
	}

	E_RESULT_CODE CSpriteRendererSystem::Init(TPtr<IAllocator> allocator, IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager)
	{
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

		mpRenderQueue = mpRenderer->GetRenderQueue(E_RENDER_QUEUE_GROUP::RQG_SPRITES);

		mpResourceManager = mpRenderer->GetResourceManager();

		mpGraphicsObjectManager = pGraphicsObjectManager;

		mpSpriteVertexDeclaration = pGraphicsObjectManager->CreateVertexDeclaration().Get();

		/// pre allocated temporary buffer for batching sprites instancing data
		_initializeBatchVertexBuffers(mpGraphicsObjectManager, PreCreatedNumOfVertexBuffers);

		/// \todo Replace this hardcoded part with proper vertex data's creationg
		mpSpriteVertexDeclaration->AddElement({ TDEngine2::FT_FLOAT4, 0, TDEngine2::VEST_POSITION });
		mpSpriteVertexDeclaration->AddElement({ TDEngine2::FT_FLOAT2, 0, TDEngine2::VEST_TEXCOORDS });
		mpSpriteVertexDeclaration->AddElement({ TDEngine2::FT_FLOAT4, 1, TDEngine2::VEST_TEXCOORDS, true });
		mpSpriteVertexDeclaration->AddElement({ TDEngine2::FT_FLOAT4, 1, TDEngine2::VEST_TEXCOORDS, true });
		mpSpriteVertexDeclaration->AddElement({ TDEngine2::FT_FLOAT4, 1, TDEngine2::VEST_TEXCOORDS, true });
		mpSpriteVertexDeclaration->AddElement({ TDEngine2::FT_FLOAT4, 1, TDEngine2::VEST_TEXCOORDS, true });
		mpSpriteVertexDeclaration->AddElement({ TDEngine2::FT_FLOAT4, 1, TDEngine2::VEST_COLOR, true });
		mpSpriteVertexDeclaration->AddInstancingDivisor(2, 1);

		TSpriteVertex vertices[] =
		{
			{ TVector4(-0.5f, 0.5f, 0.0f, 1.0f), TVector2(0.0f, 0.0f) },
			{ TVector4(0.5f, 0.5f, 0.0f, 1.0f), TVector2(1.0f, 0.0f) },
			{ TVector4(-0.5f, -0.5f, 0.0f, 1.0f), TVector2(0.0f, 1.0f) },
			{ TVector4(0.5f, -0.5f, 0.0f, 1.0f), TVector2(1.0f, 1.0f) }
		};

		mpSpriteVertexBuffer = pGraphicsObjectManager->CreateVertexBuffer(BUT_STATIC, sizeof(TSpriteVertex) * 4, &vertices[0]).Get();
		
		mpSpriteIndexBuffer = pGraphicsObjectManager->CreateIndexBuffer(BUT_STATIC, IFT_INDEX16, sizeof(U16) * 6, mSpriteFaces).Get();

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
		std::vector<TEntityId> entities = pWorld->FindEntitiesWithComponents<CTransform, CQuadSprite>();

		mTransforms.clear();		
		mSprites.clear();

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

			TDE2_ASSERT(mTransforms.back());
			TDE2_ASSERT(mSprites.back());
		}
	}

	void CSpriteRendererSystem::Update(IWorld* pWorld, F32 dt)
	{
		TDrawIndexedInstancedCommand* pCurrCommand = nullptr;

		CTransform* pCurrTransform = nullptr;

		CQuadSprite* pCurrSprite = nullptr;

		U32 groupKey = 0x0;

		/// allocate memory for vertex buffers that will store instances data if it's not allocated yet
		if (mSpritesPerInstanceData.empty())
		{
			_initializeBatchVertexBuffers(mpGraphicsObjectManager, PreCreatedNumOfVertexBuffers);
		}

		for (U32 i = 0; i < static_cast<U32>(mSprites.size()); ++i)
		{
			pCurrTransform = mTransforms[i];

			pCurrSprite = mSprites[i];
			
			const TResourceId currMaterialHandle = mpResourceManager->Load<IMaterial>(pCurrSprite->GetMaterialName());

			groupKey = _computeSpriteCommandKey(currMaterialHandle, mpGraphicsLayers->GetLayerIndex(pCurrTransform->GetPosition().z));
			
			TBatchEntry& currBatchEntry = mBatches[groupKey];

			currBatchEntry.mMaterialHandle = currMaterialHandle;

			if (!currBatchEntry.mpInstancesData)
			{
				currBatchEntry.mpInstancesData = new CDynamicArray<TSpriteInstanceData>(*mpTempAllocator.Get(), 100);
			}

			currBatchEntry.mpInstancesData->PushBack({ Transpose(pCurrTransform->GetLocalToWorldTransform()), pCurrSprite->GetColor() });
		}
		
		U32 currInstancesBufferIndex = 0;

		IVertexBuffer* pCurrBatchInstancesBuffer = nullptr;

		U32 instancesCount = 0;

		for (auto iter = mBatches.begin(); iter != mBatches.end(); ++iter)
		{
			pCurrBatchInstancesBuffer = mSpritesPerInstanceData[currInstancesBufferIndex++];

			TBatchEntry& currBatchEntry = (*iter).second;

			instancesCount    = static_cast<U32>(currBatchEntry.mpInstancesData->GetSize());
			U32 currBatchSize = instancesCount * sizeof(TSpriteInstanceData);

			if (currBatchSize <= SpriteInstanceDataBufferSize)
			{
				pCurrBatchInstancesBuffer->Map(BMT_WRITE_DISCARD);
				pCurrBatchInstancesBuffer->Write(&(*currBatchEntry.mpInstancesData)[0], currBatchSize);
				pCurrBatchInstancesBuffer->Unmap();
			}

			TPtr<IMaterial> pMaterial = mpResourceManager->GetResource<IMaterial>(currBatchEntry.mMaterialHandle);
			ITexture* pMainTexture = pMaterial->GetTextureResource(Wrench::StringUtils::GetEmptyStr());

			pCurrCommand = mpRenderQueue->SubmitDrawCommand<TDrawIndexedInstancedCommand>((*iter).first); /// \note (*iter).first is a group key that was computed before

			pCurrCommand->mpVertexBuffer           = mpSpriteVertexBuffer;
			pCurrCommand->mpIndexBuffer            = mpSpriteIndexBuffer;
			pCurrCommand->mPrimitiveType           = E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_LIST;
			pCurrCommand->mIndicesPerInstance      = 6;
			pCurrCommand->mBaseVertexIndex         = 0;
			pCurrCommand->mStartIndex              = 0;
			pCurrCommand->mStartInstance           = 0;
			pCurrCommand->mNumOfInstances          = instancesCount;/// assign number of sprites in a batch
			pCurrCommand->mpInstancingBuffer       = pCurrBatchInstancesBuffer; /// assign accumulated data of a batch
			pCurrCommand->mMaterialHandle          = currBatchEntry.mMaterialHandle;
			pCurrCommand->mpVertexDeclaration      = mpSpriteVertexDeclaration;

			auto&& uvRect = pMainTexture ? pMainTexture->GetNormalizedTextureRect() : TRectF32 { 0.0f, 0.0f, 1.0f, 1.0f};

			pCurrCommand->mObjectData.mModelMatrix = IdentityMatrix4;
			pCurrCommand->mObjectData.mTextureTransformDesc = { uvRect.x, uvRect.y, uvRect.width, uvRect.height };

			currBatchEntry.mpInstancesData->Clear();
		}
	}

	U32 CSpriteRendererSystem::_computeSpriteCommandKey(TResourceId materialId, U16 graphicsLayerId)
	{
		return static_cast<U32>(materialId) << 16 | graphicsLayerId;
	}

	void CSpriteRendererSystem::_initializeBatchVertexBuffers(IGraphicsObjectManager* pGraphicsObjectManager, U32 numOfBuffers)
	{
		for (U32 i = 0; i < numOfBuffers; ++i)
		{
			mSpritesPerInstanceData.push_back(pGraphicsObjectManager->CreateVertexBuffer(BUT_DYNAMIC, SpriteInstanceDataBufferSize, nullptr).Get());
		}
	}


	TDE2_API ISystem* CreateSpriteRendererSystem(TPtr<IAllocator> allocator, IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CSpriteRendererSystem, result, allocator, pRenderer, pGraphicsObjectManager);
	}
}