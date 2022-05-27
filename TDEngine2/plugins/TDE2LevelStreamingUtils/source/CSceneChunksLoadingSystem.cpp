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
		mContext.mpBounds.clear();

		auto mpCamerasContext = pWorld->FindEntity(pWorld->FindEntityWithUniqueComponent<CCamerasContextComponent>())->GetComponent<CCamerasContextComponent>();
		mCurrActiveCameraTransform = pWorld->FindEntity(mpCamerasContext->GetActiveCameraEntityId())->GetComponent<CTransform>();

		auto sceneLoadingTriggers = pWorld->FindEntitiesWithComponents<CSceneLoadingTriggerComponent>();
		for (TEntityId currEntityId : sceneLoadingTriggers)
		{
			if (auto pEntity = pWorld->FindEntity(currEntityId))
			{
				mContext.mpBounds.push_back((pEntity->HasComponent<CBoundsComponent>()) ? pEntity->AddComponent<CBoundsComponent>() : pEntity->GetComponent<CBoundsComponent>());
			}
		}
	}

	void CSceneChunksLoadingSystem::Update(IWorld* pWorld, F32 dt)
	{
	}


	TDE2_API ISystem* CreateSceneChunksLoadingSystem(ISceneManager* pSceneManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CSceneChunksLoadingSystem, result, pSceneManager);
	}
}