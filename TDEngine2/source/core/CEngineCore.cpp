#include "./../../include/core/CEngineCore.h"
#include "./../../include/core/IEngineSubsystem.h"
#include "./../../include/core/IWindowSystem.h"
#include "./../../include/core/IEngineListener.h"
#include "./../../include/utils/CFileLogger.h"


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

		if (_onNotifyEngineListeners(EET_ONFREE) != RC_OK)
		{
			return RC_FAIL;
		}

#if defined (_DEBUG)
		if (MainLogger->Free() != RC_OK)
		{
			return RC_FAIL;
		}
#endif

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	E_RESULT_CODE CEngineCore::Run()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		if (_onNotifyEngineListeners(EET_ONSTART) != RC_OK)
		{
			return RC_FAIL;
		}

		IWindowSystem* pWindowSystem = dynamic_cast<IWindowSystem*>(mSubsystems[EST_WINDOW]);

		if (!pWindowSystem)
		{
			return RC_FAIL;
		}

		/// \todo replace _onFrameUpdateCallback with a user defined callback
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

	E_RESULT_CODE CEngineCore::RegisterListener(IEngineListener* pListener)
	{
		if (!pListener)
		{
			return RC_INVALID_ARGS;
		}

		/// prevent duplicating instances 
		TListenersArray::const_iterator copyIter = std::find(mEngineListeners.cbegin(), mEngineListeners.cend(), pListener);

		if (copyIter != mEngineListeners.cend())
		{
			return RC_FAIL;
		}

		pListener->SetEngineInstance(this); /// inject engine's instance into the listener

		mEngineListeners.push_back(pListener);

		return RC_OK;
	}

	E_RESULT_CODE CEngineCore::UnregisterListener(IEngineListener* pListener)
	{
		if (!pListener)
		{
			return RC_INVALID_ARGS;
		}
		
		TListenersArray::iterator entityIter = std::find(mEngineListeners.begin(), mEngineListeners.end(), pListener);

		if (entityIter == mEngineListeners.cend())
		{
			return RC_FAIL;
		}

		mEngineListeners.erase(entityIter);

		return RC_OK;
	}

	IEngineSubsystem* CEngineCore::GetSubsystem(E_ENGINE_SUBSYSTEM_TYPE type) const
	{
		if (type == EST_UNKNOWN)
		{
			return nullptr;
		}

		return mSubsystems[type];
	}

	void CEngineCore::_onFrameUpdateCallback()
	{
		_onNotifyEngineListeners(EET_ONUPDATE);
	}

	E_RESULT_CODE CEngineCore::_onNotifyEngineListeners(E_ENGINE_EVENT_TYPE eventType)
	{
		if (mEngineListeners.empty())
		{
			return RC_OK;
		}

		E_RESULT_CODE resultCode = RC_OK;

		for (IEngineListener* pListener : mEngineListeners)
		{
			if (!pListener)
			{
				continue;
			}

			switch (eventType)
			{
				case EET_ONSTART:
					resultCode = pListener->OnStart();
					break;
				case EET_ONUPDATE:
					resultCode = pListener->OnUpdate(0.0f /*! \todo replace with delta time's value */);
					break;
				case EET_ONFREE:
					resultCode = pListener->OnFree();
					break;
			}

			if (resultCode != RC_OK)
			{
				return resultCode;
			}
		}

		return RC_OK;
	}
	

	IEngineCore* CreateEngineCore(E_RESULT_CODE& result)
	{
		IEngineCore* pEngineCore = new (std::nothrow) CEngineCore();

		if (!pEngineCore)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pEngineCore->Init();

		return pEngineCore;
	}
}