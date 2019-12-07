#include "./../../include/core/CEngineCore.h"
#include "./../../include/core/IEngineSubsystem.h"
#include "./../../include/core/IWindowSystem.h"
#include "./../../include/core/IEngineListener.h"
#include "./../../include/core/IEventManager.h"
#include "./../../include/core/IPlugin.h"
#include "./../../include/core/IDLLManager.h"
#include "./../../include/core/IGraphicsContext.h"
#include "./../../include/core/IInputContext.h"
#include "./../../include/core/memory/IMemoryManager.h"
#include "./../../include/core/memory/CLinearAllocator.h"
#include "./../../include/core/IImGUIContext.h"
#include "./../../include/ecs/CWorld.h"
#include "./../../include/ecs/CSpriteRendererSystem.h"
#include "./../../include/ecs/CTransformSystem.h"
#include "./../../include/ecs/CCameraSystem.h"
#include "./../../include/ecs/CPhysics2DSystem.h"
#include "./../../include/ecs/CStaticMeshRendererSystem.h"
#include "./../../include/graphics/IRenderer.h"
#include "./../../include/utils/CFileLogger.h"
#include "./../../include/utils/ITimer.h"
#include <cstring>
#include <algorithm>


namespace TDEngine2
{
	CEngineCore::CEngineCore():
		mIsInitialized(false),
		mpInternalTimer(nullptr),
		mpDLLManager(nullptr),
		mpWorldInstance(nullptr),
		mpInputContext(nullptr)
	{
	}
	
	E_RESULT_CODE CEngineCore::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		std::lock_guard<std::mutex> lock(mMutex);
		
		mIsInitialized = true;

		memset(mSubsystems, 0, sizeof(mSubsystems));
		
		LOG_MESSAGE("[Engine Core] The engine's core starts to work...");

		return RC_OK;
	}

	E_RESULT_CODE CEngineCore::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		{
			E_RESULT_CODE result = _onNotifyEngineListeners(EET_ONFREE); ///< \note call user's code

			std::lock_guard<std::mutex> lock(mMutex);

			result = result | (mpWorldInstance ? mpWorldInstance->Free() : RC_FAIL);
			result = result | _cleanUpSubsystems();

#if defined (_DEBUG)
			result = result | MainLogger->Free();
#endif
		}

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
		
		std::lock_guard<std::mutex> lock(mMutex);

		E_RESULT_CODE result = RC_OK;

		IWindowSystem* pWindowSystem = _getSubsystemAs<IWindowSystem>(EST_WINDOW);
		
		mpWorldInstance = CreateWorld(_getSubsystemAs<IEventManager>(EST_EVENT_MANAGER), result);

		if (result != RC_OK)
		{
			return result;
		}

		/// \note Try to get a pointer to IImGUIContext implementation
		mpImGUIContext = _getSubsystemAs<IImGUIContext>(EST_IMGUI_CONTEXT);
		
		/// \note we can proceed if the window wasn't initialized properly or some error has happened within user's code
		if (!pWindowSystem || (_onNotifyEngineListeners(EET_ONSTART) != RC_OK))
		{
			return RC_FAIL;
		}

		/// \todo This is not critical error, but for now I leave it as is
		if ((result = _registerBuiltinSystems(mpWorldInstance, pWindowSystem, 
											  _getSubsystemAs<IGraphicsContext>(EST_GRAPHICS_CONTEXT),
											  _getSubsystemAs<IRenderer>(EST_RENDERER),
											  _getSubsystemAs<IMemoryManager>(EST_MEMORY_MANAGER),
											  _getSubsystemAs<IEventManager>(EST_EVENT_MANAGER))) != RC_OK)
		{
			return result;
		}

		mpInternalTimer = pWindowSystem->GetTimer();

		mpInputContext = _getSubsystemAs<IInputContext>(EST_INPUT_CONTEXT);

		LOG_MESSAGE("[Engine Core] The engine's core begins to execute the main loop...");

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

		IWindowSystem* pWindowSystem = _getSubsystemAs<IWindowSystem>(EST_WINDOW);

		if (!pWindowSystem)
		{
			return RC_FAIL;
		}

		LOG_MESSAGE("[Engine Core] A signal to quit was received");

		return pWindowSystem->Quit();
	}

	E_RESULT_CODE CEngineCore::RegisterSubsystem(IEngineSubsystem* pSubsystem)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		E_ENGINE_SUBSYSTEM_TYPE subsystemType = EST_UNKNOWN;
		
		if (!pSubsystem || ((subsystemType = pSubsystem->GetType()) == EST_UNKNOWN))
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = _unregisterSubsystem(subsystemType);

		if (result != RC_OK)
		{
			return result;
		}

		mSubsystems[subsystemType] = pSubsystem;

		LOG_MESSAGE(std::string("[Engine Core] A new subsystem was successfully registered: ").append(EngineSubsystemTypeToString(pSubsystem->GetType())));

		return RC_OK;
	}

	E_RESULT_CODE CEngineCore::UnregisterSubsystem(E_ENGINE_SUBSYSTEM_TYPE subsystemType)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		return _unregisterSubsystem(subsystemType);
	}

	E_RESULT_CODE CEngineCore::RegisterListener(IEngineListener* pListener)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (!pListener)
		{
			return RC_INVALID_ARGS;
		}

		/// prevent duplicating instances
		if (std::find(mEngineListeners.cbegin(), mEngineListeners.cend(), pListener) != mEngineListeners.cend())
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
		std::lock_guard<std::mutex> lock(mMutex);

		if (!pListener)
		{
			return RC_INVALID_ARGS;
		}
		
		auto entityIter = std::find(mEngineListeners.begin(), mEngineListeners.end(), pListener);

		if (entityIter == mEngineListeners.cend())
		{
			return RC_FAIL;
		}

		mEngineListeners.erase(entityIter);

		LOG_MESSAGE("[Engine Core] The new listener was successfully unregistered");

		return RC_OK;
	}

	ILogger* CEngineCore::GetLogger() const
	{
#if defined(TDE2_DEBUG_MODE)
		return MainLogger;
#else
		return nullptr;
#endif
	}

	ITimer* CEngineCore::GetTimer() const
	{
		//std::lock_guard<std::mutex> lock(mMutex);

		return mpInternalTimer;
	}

	IWorld* CEngineCore::GetWorldInstance() const
	{
		//std::lock_guard<std::mutex> lock(mMutex);

		return mpWorldInstance;
	}

	IEngineSubsystem* CEngineCore::_getSubsystem(E_ENGINE_SUBSYSTEM_TYPE type) const
	{
		if (type == EST_UNKNOWN)
		{
			return nullptr;
		}

		//std::lock_guard<std::mutex> lock(mMutex);

		return mSubsystems[type];
	}

	void CEngineCore::_onFrameUpdateCallback()
	{
		if (mpInputContext)
		{
			mpInputContext->Update();
		}

		_onNotifyEngineListeners(EET_ONUPDATE);

		IRenderer* pRenderer = _getSubsystemAs<IRenderer>(EST_RENDERER);

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

		E_RESULT_CODE result = RC_OK;

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
					result = result | pListener->OnStart();
					break;
				case EET_ONUPDATE:
					{
						dt = mpInternalTimer->GetDeltaTime();

						if (mpImGUIContext)
						{
							mpImGUIContext->BeginFrame(dt);
						}

						/// \note The internal callback will be invoked when the execution process will go out of the scope
						CDeferOperation finalizeImGUIContextFrame([this]()
						{
							if (mpImGUIContext)
							{
								mpImGUIContext->EndFrame();
							}
						});

						mpWorldInstance->Update(dt);

						result = result | pListener->OnUpdate(dt);
					}
					break;
				case EET_ONFREE:
					result = result | pListener->OnFree();
					break;
			}
		}

		return result;
	}

	/// \todo Refactor the method

	E_RESULT_CODE CEngineCore::_registerBuiltinSystems(IWorld* pWorldInstance, IWindowSystem* pWindowSystem, IGraphicsContext* pGraphicsContext,
													   IRenderer* pRenderer, IMemoryManager* pMemoryManager, IEventManager* pEventManager)
	{
		IGraphicsObjectManager* pGraphicsObjectManager = pGraphicsContext->GetGraphicsObjectManager();

		E_RESULT_CODE result = RC_OK;

		std::vector<ISystem*> builtinSystems 
		{
			CreateTransformSystem(result),
			CreateSpriteRendererSystem(*pMemoryManager->CreateAllocator<CLinearAllocator>(5 * SpriteInstanceDataBufferSize, "sprites_batch_data"),
									   pRenderer, pGraphicsObjectManager, result),
			CreateCameraSystem(pWindowSystem, pGraphicsContext, pRenderer, result),
			CreatePhysics2DSystem(pEventManager, result),
			CreateStaticMeshRendererSystem(pRenderer, pGraphicsObjectManager, result),
		};

		for (ISystem* pCurrSystem : builtinSystems)
		{
			if (!pCurrSystem)
			{
				continue;
			}

			auto registeredSystemIdResult = pWorldInstance->RegisterSystem(pCurrSystem);

			if (registeredSystemIdResult.HasError())
			{
				LOG_ERROR("[Engine Core] Could not register system");
			}
		}

		return RC_OK;
	}

	E_RESULT_CODE CEngineCore::_unregisterSubsystem(E_ENGINE_SUBSYSTEM_TYPE subsystemType)
	{
		IEngineSubsystem* pEngineSubsystem = mSubsystems[subsystemType];

		if (!pEngineSubsystem)
		{
			return RC_OK;
		}

		mSubsystems[subsystemType] = nullptr;

		E_RESULT_CODE result = pEngineSubsystem->Free();

		const static U16 statusStringLength = 32;

		std::string subsystemName = std::move(std::string("[Engine Core] ").append(EngineSubsystemTypeToString(subsystemType)));

		MainLogger->LogStatus(subsystemName, result != RC_OK ? "FAILED" : "OK", '.', statusStringLength - subsystemName.size());

		return result;
	}

	E_RESULT_CODE CEngineCore::_cleanUpSubsystems()
	{
		E_RESULT_CODE result = RC_OK;

		LOG_MESSAGE("[Engine Core] Clean up the subsystems registry...");

		/// \note these two subsystems should be destroyed in last moment and in coresponding order
		auto latestFreedSubsystems = { EST_PLUGIN_MANAGER, EST_FILE_SYSTEM, EST_MEMORY_MANAGER, EST_WINDOW };

		auto shouldSkipSubsystem = [&latestFreedSubsystems](const IEngineSubsystem* pCurrSubsystem)
		{
			return std::find_if(latestFreedSubsystems.begin(), latestFreedSubsystems.end(), [&pCurrSubsystem](E_ENGINE_SUBSYSTEM_TYPE type)
			{
				return pCurrSubsystem->GetType() == type;
			}) != latestFreedSubsystems.end();
		};

		/// frees memory from all subsystems
		for (IEngineSubsystem* pCurrSubsystem : mSubsystems)
		{
			if (!pCurrSubsystem || shouldSkipSubsystem(pCurrSubsystem))
			{
				continue;
			}

			result = _unregisterSubsystem(pCurrSubsystem->GetType());
		}

		for (auto currSubsystemType : latestFreedSubsystems)
		{
			result = _unregisterSubsystem(currSubsystemType);
		}

		return result;
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