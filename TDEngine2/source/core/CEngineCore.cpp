#include "./../../include/core/CEngineCore.h"
#include "./../../include/core/IEngineSubsystem.h"
#include "./../../include/core/IWindowSystem.h"


namespace TDEngine2
{
	CEngineCore::CEngineCore():
		mIsInitialized(false)
	{
	}

	CEngineCore::~CEngineCore()
	{
	}

	E_RESULT_CODE CEngineCore::Init()
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		memset(&mSubsystems, 0, sizeof(mSubsystems));

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CEngineCore::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		return RC_OK;
	}

	E_RESULT_CODE CEngineCore::Run()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		IWindowSystem* pWindowSystem = dynamic_cast<IWindowSystem*>(mSubsystems[EST_WINDOW]);

		if (!pWindowSystem)
		{
			return RC_FAIL;
		}

		pWindowSystem->Run(std::bind(&CEngineCore::_onFrameUpdateCallback, this));
		
		return RC_OK;
	}

	E_RESULT_CODE CEngineCore::Quit()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		IWindowSystem* pWindowSystem = dynamic_cast<IWindowSystem*>(mSubsystems[EST_WINDOW]);

		if (!pWindowSystem)
		{
			return RC_FAIL;
		}

		if ((result = pWindowSystem->Quit()) != RC_OK)
		{
			return result;
		}

		return RC_OK;
	}

	E_RESULT_CODE CEngineCore::RegisterSubsystem(IEngineSubsystem* pSubsystem)
	{
		E_ENGINE_SUBSYSTEM_TYPE subsystemType = EST_UNKNOWN;

		if (!pSubsystem || ((subsystemType = pSubsystem->GetType()) == EST_UNKNOWN))
		{
			return RC_FAIL;
		}

		if (mSubsystems[subsystemType])
		{
			/// \todo check up is choosen slot free, if not print warning and try to free the memory it stores
		}

		mSubsystems[subsystemType] = pSubsystem;

		return RC_OK;
	}

	E_RESULT_CODE CEngineCore::UnregisterSubsystem(E_ENGINE_SUBSYSTEM_TYPE subsystemType)
	{
		return RC_OK;
	}

	void CEngineCore::_onFrameUpdateCallback()
	{
	}
}