#include "../include/CSceneChunksLoadingSystem.h"
#include "../include/CSceneLoadingTriggerComponent.h"
#include <scene/ISceneManager.h>
#include <scene/IScene.h>
#include <ecs/IWorld.h>
#include <ecs/CTransform.h>
#include <ecs/components/CBoundsComponent.h>
#include <graphics/CBaseCamera.h>
#include <editor/CPerfProfiler.h>


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
		TDE2_PROFILER_SCOPE("CSceneChunksLoadingSystem::Update");

		auto& sceneLoadingTriggers = std::get<std::vector<CSceneLoadingTriggerComponent*>>(mContext.mComponentsSlice);
		auto& bounds               = std::get<std::vector<CBoundsComponent*>>(mContext.mComponentsSlice);
		auto& transforms           = std::get<std::vector<CTransform*>>(mContext.mComponentsSlice);

		/// \note First step is updating of all bounds
		for (USIZE i = 0; i < mContext.mComponentsCount; ++i)
		{
			auto&& pSceneLoadingTrigger = sceneLoadingTriggers[i];

			const TVector3& sizes = pSceneLoadingTrigger->GetVolumeSizes();

			const TAABB boundingVolume = pSceneLoadingTrigger->IsDirty() ? 
													TAABB(transforms[i]->GetPosition() + pSceneLoadingTrigger->GetVolumeOffset(), sizes.x, sizes.y, sizes.z) : 
													bounds[i]->GetBounds();

			if (pSceneLoadingTrigger->IsDirty())
			{
				bounds[i]->SetBounds(boundingVolume);
				pSceneLoadingTrigger->SetDirtyFlag(false);
			}

			const bool prevOverlappingState = pSceneLoadingTrigger->GetOverlappingState();
			const bool currOverlappingState = ContainsPoint(boundingVolume, mCurrActiveCameraTransform->GetPosition());

			if (currOverlappingState && !prevOverlappingState) // enter the volume
			{
				auto&& scenePath = pSceneLoadingTrigger->GetScenePath();

				if (!scenePath.empty())
				{
					mpSceneManager->LoadSceneAsync(pSceneLoadingTrigger->GetScenePath(), nullptr);	/// \note Load scene's chunk
				}

				pSceneLoadingTrigger->SetOverlappingState(true);
			}
			else if (prevOverlappingState && !currOverlappingState) // goes out of the volume
			{				
				const TSceneId sceneHandle = mpSceneManager->GetSceneId(pSceneLoadingTrigger->GetScenePath());
				if (TSceneId::Invalid != sceneHandle)
				{
					mpSceneManager->UnloadScene(sceneHandle); /// \note Unload it
				}

				pSceneLoadingTrigger->SetOverlappingState(false);
			}
		}
	}


	TDE2_API ISystem* CreateSceneChunksLoadingSystem(ISceneManager* pSceneManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CSceneChunksLoadingSystem, result, pSceneManager);
	}
}