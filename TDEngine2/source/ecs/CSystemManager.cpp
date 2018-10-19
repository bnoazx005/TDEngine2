#include "./../../include/ecs/CSystemManager.h"
#include "./../../include/ecs/ISystem.h"
#include "./../../include/ecs/IWorld.h"
#include <algorithm>


namespace TDEngine2
{
	CSystemManager::CSystemManager() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CSystemManager::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CSystemManager::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		/// \todo add memory release code

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	E_RESULT_CODE CSystemManager::RegisterSystem(ISystem* pSystem)
	{
		if (!pSystem)
		{
			return RC_INVALID_ARGS;
		}

		auto duplicateIter = std::find(mpActiveSystems.begin(), mpActiveSystems.end(), pSystem);

		if (duplicateIter != mpActiveSystems.end()) /// if there is a duplicate, just interrupt registration process
		{
			return RC_FAIL;
		}

		mpActiveSystems.push_back(pSystem);

		return RC_OK;
	}
	
	E_RESULT_CODE CSystemManager::UnregisterSystem(ISystem* pSystem)
	{
		if (!pSystem)
		{
			return RC_INVALID_ARGS;
		}

		auto targetSystemIter = std::find(mpActiveSystems.begin(), mpActiveSystems.end(), pSystem);

		if (targetSystemIter == mpActiveSystems.end()) /// specified system is not registred yet
		{
			return RC_FAIL;
		}

		mpActiveSystems.erase(targetSystemIter);

		return RC_OK;
	}

	E_RESULT_CODE CSystemManager::UnregisterSystemImmediately(ISystem* pSystem)
	{
		E_RESULT_CODE result = UnregisterSystem(pSystem);

		if (result != RC_OK)
		{
			return result;
		}

		return pSystem->Free();
	}

	E_RESULT_CODE CSystemManager::ActivateSystem(ISystem* pSystem)
	{
		auto tagretSystemIter = std::find(mpDeactivatedSystems.begin(), mpDeactivatedSystems.end(), pSystem);

		if (tagretSystemIter == mpDeactivatedSystems.end())
		{
			return RC_FAIL;
		}

		mpActiveSystems.push_back(pSystem);

		mpDeactivatedSystems.erase(tagretSystemIter);

		return RC_OK;
	}

	E_RESULT_CODE CSystemManager::DeactivateSystem(ISystem* pSystem)
	{
		auto targetSystemIter = std::find(mpActiveSystems.begin(), mpActiveSystems.end(), pSystem);

		if (targetSystemIter == mpActiveSystems.end())
		{
			return RC_FAIL;
		}

		mpDeactivatedSystems.push_back(pSystem);
		mpActiveSystems.erase(targetSystemIter);

		return RC_OK;
	}

	void CSystemManager::Update(IWorld* pWorld, float dt)
	{
		for (ISystem* pCurrSystem : mpActiveSystems)
		{
			pCurrSystem->Update(pWorld, dt);
		}
	}


	ISystemManager* CreateSystemManager(E_RESULT_CODE& result)
	{
		CSystemManager* pSysManager = new (std::nothrow) CSystemManager();

		if (!pSysManager)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pSysManager->Init();

		if (result != RC_OK)
		{
			delete pSysManager;

			pSysManager = nullptr;
		}

		return dynamic_cast<ISystemManager*>(pSysManager);
	}
}