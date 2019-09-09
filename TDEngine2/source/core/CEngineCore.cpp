#include "./../../include/core/CEngineCore.h"
#include "./../../include/core/IEngineSubsystem.h"
#include "./../../include/core/IWindowSystem.h"
#include "./../../include/core/IEngineListener.h"
#include "./../../include/utils/CFileLogger.h"
#include "./../../include/core/IPlugin.h"
#include "./../../include/core/IDLLManager.h"
#include "./../../include/utils/ITimer.h"
#include "./../../include/ecs/CWorld.h"
#include "./../../include/core/IEventManager.h"
#include "./../../include/ecs/CSpriteRendererSystem.h"
#include "./../../include/ecs/CTransformSystem.h"
#include "./../../include/graphics/IRenderer.h"
#include "./../../include/core/IGraphicsContext.h"
#include "./../../include/core/IInputContext.h"
#include "./../../include/ecs/CCameraSystem.h"
#include "./../../include/ecs/CPhysics2DSystem.h"
#include <cstring>
#include <algorithm>


namespace TDEngine2
{
	CEngineCore::CEngineCore():
		mIsInitialized(false)
	{
	}
	
	E_RESULT_CODE CEngineCore::Init()
	{
		//std::lock_guard<std::mutex> lock(mMutex);

		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		memset(&mSubsystems, 0, sizeof(mSubsystems));

		mpDLLManager = nullptr;
		
		mIsInitialized = true;
		
		LOG_MESSAGE("[Engine Core] The engine's core starts to work...");

		return RC_OK;
	}

	E_RESULT_CODE CEngineCore::Free()
	{
		//std::lock_guard<std::mutex> lock(mMutex);

		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		if (_onNotifyEngineListeners(EET_ONFREE) != RC_OK)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		if (mpWorldInstance && (result = mpWorldInstance->Free()) != RC_OK)
		{
			return result;
		}
		
		LOG_MESSAGE("[Engine Core] Clear the subsystems registry...");

		/// frees memory from all subsystems
		E_ENGINE_SUBSYSTEM_TYPE currSubsystemType;

		for (IEngineSubsystem* pCurrSubsystem : mSubsystems)
		{
			if (!pCurrSubsystem)
			{
				continue;
			}

			currSubsystemType = pCurrSubsystem->GetType();

			if (currSubsystemType == EST_PLUGIN_MANAGER)
			{
				continue;
			}

			if ((result = UnregisterSubsystem(currSubsystemType)) != RC_OK)
			{
				return result;
			}
		}

		if ((result = UnregisterSubsystem(EST_PLUGIN_MANAGER)) != RC_OK)
		{
			return result;
		}

#if defined (_DEBUG)
		if (MainLogger->Free() != RC_OK)
		{
			return RC_FAIL;
		}
#endif

		mIsInitialized = false;

		delete this;

		LOG_MESSAGE("[Engine Core] The engine's core was successfully finalized");

		return RC_OK;
	}

	E_RESULT_CODE CEngineCore::Run()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;
		
		IEventManager* pEventManager = dynamic_cast<IEventManager*>(mSubsystems[EST_EVENT_MANAGER]);

		mpWorldInstance = CreateWorld(pEventManager, result);

		if (result != RC_OK)
		{
			return result;
		}

		IWindowSystem* pWindowSystem = dynamic_cast<IWindowSystem*>(mSubsystems[EST_WINDOW]);

		if (!pWindowSystem)
		{
			return RC_FAIL;
		}
		
		if (_onNotifyEngineListeners(EET_ONSTART) != RC_OK)
		{
			return RC_FAIL;
		}

		if ((result = _registerBuiltinSystems(mpWorldInstance, pWindowSystem, dynamic_cast<IGraphicsContext*>(mSubsystems[EST_GRAPHICS_CONTEXT]),
			dynamic_cast<IRenderer*>(mSubsystems[EST_RENDERER]))) != RC_OK)
		{
			return result;
		}

		mpInternalTimer = pWindowSystem->GetTimer();

		mpInputContext = dynamic_cast<IInputContext*>(mSubsystems[EST_INPUT_CONTEXT]);

		LOG_MESSAGE("[Engine Core] The engine's core begins to execute the main loop...");

		/// \todo replace _onFrameUpdateCallback with a user defined callback
		pWindowSystem->Run(std::bind(&CEngineCore::_onFrameUpdateCallback, this));
		
		return RC_OK;
	}

	E_RESULT_CODE CEngineCore::Quit()
	{
		//std::lock_guard<std::mutex> lock(mMutex);

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

		LOG_MESSAGE("[Engine Core] A signal to quit was received");

		return RC_OK;
	}

	E_RESULT_CODE CEngineCore::RegisterSubsystem(IEngineSubsystem* pSubsystem)
	{
		//std::lock_guard<std::mutex> lock(mMutex);

		E_ENGINE_SUBSYSTEM_TYPE subsystemType = EST_UNKNOWN;
		
		if (!pSubsystem || ((subsystemType = pSubsystem->GetType()) == EST_UNKNOWN))
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = _unregisterSubsystem(subsystemType);

		if (result != RC_OK && mSubsystems[subsystemType])
		{
			return result;
		}

		mSubsystems[subsystemType] = pSubsystem;
		
		LOG_MESSAGE("[Engine Core] A new subsystem was successfully registered");

		return RC_OK;
	}

	E_RESULT_CODE CEngineCore::UnregisterSubsystem(E_ENGINE_SUBSYSTEM_TYPE subsystemType)
	{
		//std::lock_guard<std::mutex> lock(mMutex);

		return _unregisterSubsystem(subsystemType);
	}

	E_RESULT_CODE CEngineCore::RegisterListener(IEngineListener* pListener)
	{
		//std::lock_guard<std::mutex> lock(mMutex);

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

		LOG_MESSAGE("[Engine Core] A new listener was successfully registered");

		return RC_OK;
	}

	E_RESULT_CODE CEngineCore::UnregisterListener(IEngineListener* pListener)
	{
		//std::lock_guard<std::mutex> lock(mMutex);

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

		LOG_MESSAGE("[Engine Core] The new listener was successfully unregistered");

		return RC_OK;
	}

	IEngineSubsystem* CEngineCore::GetSubsystem(E_ENGINE_SUBSYSTEM_TYPE type) const
	{
		//std::lock_guard<std::mutex> lock(mMutex);

		if (type == EST_UNKNOWN)
		{
			return nullptr;
		}

		return mSubsystems[type];
	}

	ILogger* CEngineCore::GetLogger() const
	{
		//std::lock_guard<std::mutex> lock(mMutex);

#if defined (_DEBUG)
		return MainLogger;
#else
		return nullptr;
#endif
	}

	ITimer* CEngineCore::GetTimer() const
	{
		//std::lock_guard<std::mutex> lock(mMutex);

		IWindowSystem* pWindowSystem = dynamic_cast<IWindowSystem*>(mSubsystems[EST_WINDOW]);
		
		if (!pWindowSystem)
		{
			return nullptr;
		}

		return pWindowSystem->GetTimer();
	}

	IWorld* CEngineCore::GetWorldInstance() const
	{
		return mpWorldInstance;
	}

	void CEngineCore::_onFrameUpdateCallback()
	{
		if (mpInputContext)
		{
			mpInputContext->Update();
		}

		_onNotifyEngineListeners(EET_ONUPDATE);

		IRenderer* pRenderer = dynamic_cast<IRenderer*>(mSubsystems[EST_RENDERER]);

		if (pRenderer)
		{
			pRenderer->Draw();
		}
	}

	E_RESULT_CODE CEngineCore::_onNotifyEngineListeners(E_ENGINE_EVENT_TYPE eventType)
	{
		if (mEngineListeners.empty())
		{
			return RC_OK;
		}

		E_RESULT_CODE resultCode = RC_OK;

		F32 dt = 0.0f;

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
					dt = mpInternalTimer->GetDeltaTime();

					mpWorldInstance->Update(dt);

					resultCode = pListener->OnUpdate(dt);
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

	E_RESULT_CODE CEngineCore::_unregisterSubsystem(E_ENGINE_SUBSYSTEM_TYPE subsystemType)
	{
		IEngineSubsystem* pEngineSubsystem = mSubsystems[subsystemType];

		if (!pEngineSubsystem)
		{
			return RC_FAIL;
		}

		mSubsystems[subsystemType] = nullptr;

		LOG_MESSAGE("[Engine Core] The subsystem was successfully unregistered");

		return pEngineSubsystem->Free();
	}

	E_RESULT_CODE CEngineCore::_registerBuiltinSystems(IWorld* pWorldInstance, IWindowSystem* pWindowSystem, IGraphicsContext* pGraphicsContext,
													   IRenderer* pRenderer)
	{
		IGraphicsObjectManager* pGraphicsObjectManager = pGraphicsContext->GetGraphicsObjectManager();

		E_RESULT_CODE result = RC_OK;

		ISystem* pTransformUpdateSystem = CreateTransformSystem(result);

		if (result != RC_OK)
		{
			return result;
		}

		TResult<TSystemId> transformUpdateSystemIdResult = pWorldInstance->RegisterSystem(pTransformUpdateSystem);

		if (transformUpdateSystemIdResult.HasError())
		{
			return transformUpdateSystemIdResult.GetError();
		}

		ISystem* pSpriteRendererSystem = CreateSpriteRendererSystem(pRenderer, pGraphicsObjectManager, result);

		if (result != RC_OK)
		{
			return result;
		}

		TResult<TSystemId> spriteRenderSystemIdResult = pWorldInstance->RegisterSystem(pSpriteRendererSystem);

		if (spriteRenderSystemIdResult.HasError())
		{
			return spriteRenderSystemIdResult.GetError();
		}

		ISystem* pCameraSystem = CreateCameraSystem(pWindowSystem, pGraphicsContext, pRenderer, result);

		if (result != RC_OK)
		{
			return result;
		}

		TResult<TSystemId> cameraSystemIdResult = pWorldInstance->RegisterSystem(pCameraSystem);

		if (cameraSystemIdResult.HasError())
		{
			return cameraSystemIdResult.GetError();
		}

		ISystem* pPhysics2DSystem = CreatePhysics2DSystem(result);

		if (result != RC_OK)
		{
			return result;
		}

		TResult<TSystemId> physics2DSystemIdResult = pWorldInstance->RegisterSystem(pPhysics2DSystem);

		if (physics2DSystemIdResult.HasError())
		{
			return physics2DSystemIdResult.GetError();
		}

		return RC_OK;
	}
	

	IEngineCore* CreateEngineCore(E_RESULT_CODE& result)
	{
		CEngineCore* pEngineCore = new (std::nothrow) CEngineCore();

		if (!pEngineCore)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pEngineCore->Init();

		if (result != RC_OK)
		{
			delete pEngineCore;

			pEngineCore = nullptr;
		}

		return dynamic_cast<IEngineCore*>(pEngineCore);
	}
}