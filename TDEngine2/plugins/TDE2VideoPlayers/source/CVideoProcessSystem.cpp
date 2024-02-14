#include "../include/CVideoProcessSystem.h"
#include "../include/CUIVideoContainerComponent.h"
#include <core/IResourceManager.h>
#include <ecs/IWorld.h>
#include <graphics/UI/CImageComponent.h>
#include <editor/CPerfProfiler.h>
#include "../deps/theoraplay/theoraplay.h"


namespace TDEngine2
{
	CVideoProcessSystem::CVideoProcessSystem() :
		CBaseSystem()
	{
	}

	E_RESULT_CODE CVideoProcessSystem::Init(IResourceManager* pResourceManager)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pResourceManager)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = pResourceManager;

		mIsInitialized = true;

		return RC_OK;
	}

	void CVideoProcessSystem::InjectBindings(IWorld* pWorld)
	{
		mVideoContainers.clear();
		mVideoReceivers.clear();

		CImage* pImage = nullptr;

		U32 counter = 0;

		for (const TEntityId currEntityId : pWorld->FindEntitiesWithComponents<CUIVideoContainerComponent>())
		{
			CEntity* pEntity = pWorld->FindEntity(currEntityId);

			mVideoContainers.push_back(pEntity->GetComponent<CUIVideoContainerComponent>());

			pImage = pEntity->GetComponent<CImage>();
			if (!pImage)
			{
				pImage = pEntity->AddComponent<CImage>();

				// \todo Create a video texture
			}

			mVideoReceivers.push_back(pImage);
		}
	}

	void CVideoProcessSystem::Update(IWorld* pWorld, F32 dt)
	{	
		TDE2_PROFILER_SCOPE("CVideoProcessSystem::Update");

		for (USIZE i = 0; i < mVideoContainers.size(); i++)
		{

		}
	}


	TDE2_API ISystem* CreateVideoProcessSystem(IResourceManager* pResourceManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CVideoProcessSystem, result, pResourceManager);
	}
}