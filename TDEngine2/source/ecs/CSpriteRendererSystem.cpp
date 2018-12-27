#include "./../../include/ecs/CSpriteRendererSystem.h"
#include "./../../include/ecs/IWorld.h"
#include "./../../include/graphics/CQuadSprite.h"
#include "./../../include/ecs/CEntity.h"
#include "./../../include/ecs/CTransform.h"
#include "./../../include/graphics/CRenderQueue.h"
#include "./../../include/graphics/IGraphicsObjectManager.h"
#include "./../../include/graphics/IVertexBuffer.h"
#include "./../../include/graphics/IIndexBuffer.h"
#include "./../../include/graphics/IVertexDeclaration.h"
#include "./../../include/graphics/CGraphicsLayersInfo.h"
#include "./../../include/core/IGraphicsContext.h"
#include "./../../include/graphics/IRenderer.h"
#include "./../../include/core/IResourceHandler.h"
#include "./../../include/core/IResourceManager.h"


namespace TDEngine2
{
	CSpriteRendererSystem::CSpriteRendererSystem() :
		CBaseObject(), mpRenderQueue(nullptr), mpSpriteVertexBuffer(nullptr), mpSpriteIndexBuffer(nullptr),
		mpSpriteVertexDeclaration(nullptr), mSpriteFaces {0, 1, 2, 2, 1, 3}, mpGraphicsLayers(nullptr)
	{
	}

	E_RESULT_CODE CSpriteRendererSystem::Init(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager)
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

		mpRenderQueue = mpRenderer->GetRenderQueue(E_RENDER_QUEUE_GROUP::RQG_SPRITES);

		mpResourceManager = mpRenderer->GetResourceManager();

		mpGraphicsObjectManager = pGraphicsObjectManager;

		mpSpriteVertexDeclaration = pGraphicsObjectManager->CreateVertexDeclaration().Get();

		/// pre allocated temporary buffer for batching sprites instancing data
		for (U32 i = 0; i < PreCreatedNumOfVertexBuffers; ++i)
		{
			mSpritesPerInstanceData.push_back(pGraphicsObjectManager->CreateVertexBuffer(BUT_DYNAMIC, SpriteInstanceDataBufferSize, nullptr).Get());
		}

		/// \todo Replace this hardcoded part with proper vertex data's creationg
		mpSpriteVertexDeclaration->AddElement({ TDEngine2::FT_FLOAT4, 0, TDEngine2::VEST_POSITION });

		TVector4 vertices[] =
		{
			TVector4(-0.5f, 0.5f, 0.0f, 1.0f),
			TVector4(0.5f, 0.5f, 0.0f, 1.0f),
			TVector4(-0.5f, -0.5f, 0.0f, 1.0f),
			TVector4(0.5f, -0.5f, 0.0f, 1.0f)
		};

		mpSpriteVertexBuffer = pGraphicsObjectManager->CreateVertexBuffer(BUT_STATIC, sizeof(TVector4) * 4, &vertices[0]).Get();
		
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

	E_RESULT_CODE CSpriteRendererSystem::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		if ((result = mpGraphicsLayers->Free()) != RC_OK)
		{
			return result;
		}

		mIsInitialized = false;

		delete this;

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
		}
	}

	void CSpriteRendererSystem::Update(IWorld* pWorld, F32 dt)
	{
		TDrawIndexedInstancedCommand* pCurrCommand = nullptr;

		CTransform* pCurrTransform = nullptr;

		CQuadSprite* pCurrSprite = nullptr;

		U32 groupKey = 0x0;
		
		IResourceHandler* pCurrMaterialHandler = nullptr;

		std::string currMaterialName;

		for (I32 i = 0; i < mSprites.size(); ++i)
		{
			pCurrTransform = mTransforms[i];

			pCurrSprite = mSprites[i];

			/// accumulate instancing data

			/// if current vertex buffer is filled up send it to the renderer
			
			currMaterialName = pCurrSprite->GetMaterialName();

			groupKey = _computeSpriteCommandKey(mpResourceManager->GetResourceId(currMaterialName), mpGraphicsLayers->GetLayerIndex(pCurrTransform->GetPosition().z));

			/*pCurrCommand = mpRenderQueue->SubmitDrawCommand<TDrawIndexedInstancedCommand>(groupKey);

			pCurrCommand->mpVertexBuffer      = mpSpriteVertexBuffer;
			pCurrCommand->mpIndexBuffer       = mpSpriteIndexBuffer;
			pCurrCommand->mPrimitiveType      = E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_LIST;
			pCurrCommand->mIndicesPerInstance = 6;
			pCurrCommand->mBaseVertexIndex    = 0;
			pCurrCommand->mStartIndex         = 0;
			pCurrCommand->mStartInstance      = 0;
			pCurrCommand->mNumOfInstances     = 0;/// assign number of sprites in a batch
			pCurrCommand->mpInstancingBuffer  = nullptr; /// assign accumulated data of a batch*/

			TDrawIndexedCommand* pCurrCommand = mpRenderQueue->SubmitDrawCommand<TDrawIndexedCommand>(groupKey);

			pCurrCommand->mpVertexBuffer      = mpSpriteVertexBuffer;
			pCurrCommand->mpIndexBuffer       = mpSpriteIndexBuffer;
			pCurrCommand->mPrimitiveType      = E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_LIST;
			pCurrCommand->mStartIndex         = 0;
			pCurrCommand->mNumOfIndices       = 6;
			pCurrCommand->mpVertexDeclaration = mpSpriteVertexDeclaration;
			pCurrCommand->mMaterialName       = currMaterialName;
		}
		
		/*
		pseudocode of the logic:

		foreach(IGraphicsLayer pCurrLayer in mGraphicsLayers) {
			ISprite[] pSprites = FindSpritesWithLayer(pCurrLayer);

			pSprites = SortByMaterials(pSprites);

			foreach(ISprite pCurrSprite in pSprites) {
				DrawBatchIfChanged(pSprite->mMaterial);

				AddToBatch(pSprite);
			}
		}*/
	}

	U32 CSpriteRendererSystem::_computeSpriteCommandKey(TResourceId materialId, U16 graphicsLayerId)
	{
		return materialId << 16 | graphicsLayerId;
	}


	TDE2_API ISystem* CreateSpriteRendererSystem(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result)
	{
		CSpriteRendererSystem* pSystemInstance = new (std::nothrow) CSpriteRendererSystem();

		if (!pSystemInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pSystemInstance->Init(pRenderer, pGraphicsObjectManager);

		if (result != RC_OK)
		{
			delete pSystemInstance;

			pSystemInstance = nullptr;
		}

		return dynamic_cast<ISystem*>(pSystemInstance);
	}
}