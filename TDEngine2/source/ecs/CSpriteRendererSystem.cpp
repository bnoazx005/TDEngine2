#include "./../../include/ecs/CSpriteRendererSystem.h"
#include "./../../include/ecs/IWorld.h"
#include "./../../include/graphics/CQuadSprite.h"
#include "./../../include/ecs/CEntity.h"
#include "./../../include/ecs/CTransform.h"


namespace TDEngine2
{
	CSpriteRendererSystem::CSpriteRendererSystem() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CSpriteRendererSystem::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
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

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	void CSpriteRendererSystem::Update(IWorld* pWorld, F32 dt)
	{
		pWorld->ForEach<CQuadSprite>([&pWorld](TEntityId entityId, IComponent* pComponent)
		{
			CEntity* pEntity = pWorld->FindEntity(entityId);

			if (!pEntity)
			{
				return;
			}

			CQuadSprite* pSprite = dynamic_cast<CQuadSprite*>(pComponent);

			CTransform* pTransform = pEntity->GetComponent<CTransform>();

			// send sprite to one of batches based on (graphics layer / static / etc)
		});


		/// \todo implement this method

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


	TDE2_API ISystem* CreateSpriteRendererSystem(E_RESULT_CODE& result)
	{
		CSpriteRendererSystem* pSystemInstance = new (std::nothrow) CSpriteRendererSystem();

		if (!pSystemInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pSystemInstance->Init();

		if (result != RC_OK)
		{
			delete pSystemInstance;

			pSystemInstance = nullptr;
		}

		return dynamic_cast<ISystem*>(pSystemInstance);
	}
}