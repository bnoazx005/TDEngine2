#include "./../../include/ecs/CStaticMeshRendererSystem.h"
#include "./../../include/graphics/IGraphicsObjectManager.h"
#include "./../../include/graphics/IRenderer.h"
#include "./../../include/graphics/CStaticMeshContainer.h"
#include "./../../include/graphics/CRenderQueue.h"
#include "./../../include/ecs/CTransform.h"
#include "./../../include/ecs/IWorld.h"
#include "./../../include/ecs/CEntity.h"


namespace TDEngine2
{
	CStaticMeshRendererSystem::CStaticMeshRendererSystem() :
		CBaseObject()
	{
	}

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

		mpOpaqueRenderGroup      = pRenderer->GetRenderQueue(E_RENDER_QUEUE_GROUP::RQG_OPAQUE_GEOMETRY);
		mpTransparentRenderGroup = pRenderer->GetRenderQueue(E_RENDER_QUEUE_GROUP::RQG_TRANSPARENT_GEOMETRY);

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CStaticMeshRendererSystem::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	void CStaticMeshRendererSystem::InjectBindings(IWorld* pWorld)
	{
		std::vector<TEntityId> entities = pWorld->FindEntitiesWithComponents<CTransform, CStaticMeshContainer>();

		mProcessingEntities.clear();

		CEntity* pCurrEntity = nullptr;

		for (auto iter = entities.begin(); iter != entities.end(); ++iter)
		{
			pCurrEntity = pWorld->FindEntity(*iter);

			if (!pCurrEntity)
			{
				continue;
			}

			mProcessingEntities.push_back({ pCurrEntity->GetComponent<CTransform>(), pCurrEntity->GetComponent<CStaticMeshContainer>() });
		}
	}

	void CStaticMeshRendererSystem::Update(IWorld* pWorld, F32 dt)
	{
		for (auto& iter : mProcessingEntities)
		{
			// TODO:
			;
		}
	}


	TDE2_API ISystem* CreateStaticMeshRendererSystem(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result)
	{
		CStaticMeshRendererSystem* pSystemInstance = new (std::nothrow) CStaticMeshRendererSystem();

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