#include "../include/CSceneChunksLoadingSystem.h"
#include "../include/CSceneLoadingTriggerComponent.h"
#include <scene/ISceneManager.h>
#include <ecs/IWorld.h>
#include <ecs/CTransform.h>
#include <ecs/components/CBoundsComponent.h>
#include <graphics/CBaseCamera.h>


namespace TDEngine2
{
	CSceneChunksLoadingSystem::CSceneChunksLoadingSystem() :
		CBaseSystem()
	{
	}

	E_RESULT_CODE CSceneChunksLoadingSystem::Init(ISceneManager* pSceneManager)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pSceneManager)
		{
			return RC_INVALID_ARGS;
		}

		mpSceneManager = pSceneManager;

		mIsInitialized = true;

		return RC_OK;
	}

	void CSceneChunksLoadingSystem::InjectBindings(IWorld* pWorld)
	{
		auto mpCamerasContext = pWorld->FindEntity(pWorld->FindEntityWithUniqueComponent<CCamerasContextComponent>())->GetComponent<CCamerasContextComponent>();
		mCurrActiveCameraTransform = pWorld->FindEntity(mpCamerasContext->GetActiveCameraEntityId())->GetComponent<CTransform>();

		auto sceneLoadingTriggers = pWorld->FindEntitiesWithComponents<CSceneLoadingTriggerComponent>();
		if (sceneLoadingTriggers.empty())
		{
			return;
		}

		for (TEntityId currEntityId : sceneLoadingTriggers)
		{
			if (auto pEntity = pWorld->FindEntity(currEntityId))
			{
				if (!pEntity->HasComponent<CBoundsComponent>())
				{
					pEntity->AddComponent<CBoundsComponent>(); // \note This code will add CBoundsComponent for each CSceneLoadingTriggerComponent
				}
			}
		}

		// \note Now we can create local slice of components to work with
		mContext = pWorld->CreateLocalComponentsSlice<CSceneLoadingTriggerComponent, CBoundsComponent, CTransform>();
	}

	void CSceneChunksLoadingSystem::Update(IWorld* pWorld, F32 dt)
	{

	}


	TDE2_API ISystem* CreateSceneChunksLoadingSystem(ISceneManager* pSceneManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CSceneChunksLoadingSystem, result, pSceneManager);
	}
}