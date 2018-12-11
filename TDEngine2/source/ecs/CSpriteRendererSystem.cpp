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


namespace TDEngine2
{
	CSpriteRendererSystem::CSpriteRendererSystem() :
		CBaseObject(), mpRenderQueue(nullptr), mpSpriteVertexBuffer(nullptr), mpSpriteIndexBuffer(nullptr),
		mpSpriteVertexDeclaration(nullptr), mSpriteFaces {0, 1, 2, 2, 1, 3}, mpGraphicsLayers(nullptr)
	{
	}

	E_RESULT_CODE CSpriteRendererSystem::Init(IGraphicsObjectManager* pGraphicsObjectManager, CRenderQueue* pRenderQueue)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pRenderQueue || !pGraphicsObjectManager)
		{
			return RC_INVALID_ARGS;
		}

		mpRenderQueue = pRenderQueue;

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
		TDrawIndexedInstancedCommandPtr pCurrCommand = nullptr;

		CTransform* pCurrTransform = nullptr;

		CQuadSprite* pCurrSprite = nullptr;

		for (I32 i = 0; i < mSprites.size(); ++i)
		{
			pCurrTransform = mTransforms[i];

			pCurrSprite = mSprites[i];

			/// accumulate instancing data

			/// if current vertex buffer is filled up send it to the renderer

			pCurrCommand = mpRenderQueue->SubmitDrawCommand<TDrawIndexedInstancedCommand>(0); /// \todo replace 0 with correct computation of a group's identifier
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


	TDE2_API ISystem* CreateSpriteRendererSystem(IGraphicsObjectManager* pGraphicsObjectManager, CRenderQueue* pRenderQueue, E_RESULT_CODE& result)
	{
		CSpriteRendererSystem* pSystemInstance = new (std::nothrow) CSpriteRendererSystem();

		if (!pSystemInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pSystemInstance->Init(pGraphicsObjectManager, pRenderQueue);

		if (result != RC_OK)
		{
			delete pSystemInstance;

			pSystemInstance = nullptr;
		}

		return dynamic_cast<ISystem*>(pSystemInstance);
	}
}