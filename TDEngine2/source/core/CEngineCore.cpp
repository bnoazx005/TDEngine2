#include "../../include/core/CEngineCore.h"
#include "../../include/core/IEngineSubsystem.h"
#include "../../include/core/IWindowSystem.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/core/IEngineListener.h"
#include "../../include/core/IEventManager.h"
#include "../../include/core/IPlugin.h"
#include "../../include/core/IDLLManager.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/core/IInputContext.h"
#include "../../include/core/memory/CLinearAllocator.h"
#include "../../include/core/IImGUIContext.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/core/IAudioContext.h"
#include "../../include/core/IPluginManager.h"
#include "../../include/core/IJobManager.h"
#include "../../include/core/IGameModesManager.h"
#include "../../include/ecs/CWorld.h"
#include "../../include/ecs/CSpriteRendererSystem.h"
#include "../../include/ecs/CTransformSystem.h"
#include "../../include/ecs/CCameraSystem.h"
#include "../../include/ecs/CPhysics2DSystem.h"
#include "../../include/ecs/CStaticMeshRendererSystem.h"
#include "../../include/ecs/CSkinnedMeshRendererSystem.h"
#include "../../include/ecs/CAnimationSystem.h"
#include "../../include/ecs/CParticlesSimulationSystem.h"
#include "../../include/ecs/CUIElementsProcessSystem.h"
#include "../../include/ecs/CUIElementsRenderSystem.h"
#include "../../include/ecs/CUIEventsSystem.h"
#include "../../include/ecs/CMeshAnimatorUpdatingSystem.h"
#include "../../include/ecs/CLODMeshSwitchSystem.h"
#include "../../include/ecs/CWeatherSystem.h"
#include "../../include/scene/CSceneManager.h"
#include "../../include/scene/IScene.h"
#include "../../include/graphics/IRenderer.h"
#include "../../include/graphics/IGraphicsObjectManager.h"
#include "../../include/graphics/IDebugUtility.h"
#include "../../include/graphics/CBaseCamera.h"
#include "../../include/graphics/CFramePacketsStorage.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/utils/ITimer.h"
#define META_IMPLEMENTATION
#define META_EXPORT_CORE_SECTION
#define META_EXPORT_GRAPHICS_SECTION
#define META_EXPORT_UI_SECTION
#define META_EXPORT_ANIMATION_SECTION
#include "../../include/metadata.h"

#if TDE2_EDITORS_ENABLED
#include "../../include/editor/IEditorsManager.h"
#include "../../include/editor/CPerfProfiler.h"
#include "../../include/editor/CMemoryProfiler.h"
#endif

#include "../../include/physics/CBaseRaycastContext.h"
#include "../../include/ecs/CObjectsSelectionSystem.h"
#include "../../include/ecs/CBoundsUpdatingSystem.h"
#include "../../include/ecs/CLightingSystem.h"
#include <cstring>
#include <algorithm>
#include "deferOperation.hpp"

#if TDE2_EDITORS_ENABLED

#ifdef TDE2_USEWINPLATFORM
	#include "optick.h"
#endif

#include "tracy/Tracy.hpp"

#endif


namespace TDEngine2
{
	CEngineCore::CEngineCore():
		CBaseObject()
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

		LOG_MESSAGE("[Engine Core] >>>=================================================");
		LOG_MESSAGE("[Engine Core] The engine's core starts to work...");
		LOG_MESSAGE("[Engine Core] <<<=================================================\n");

		return RC_OK;
	}

	E_RESULT_CODE CEngineCore::_onFreeInternal()
	{
		{
			E_RESULT_CODE result = _onNotifyEngineListeners(EET_ONFREE); ///< \note call user's code

			std::lock_guard<std::mutex> lock(mMutex);

			// Unsubscribe all systems and managers within mpWorldInstance
			if (mpWorldInstance)
			{
				result = result | mpWorldInstance->OnBeforeFree();
				mpWorldInstance = nullptr;
			}

			mpInternalTimer = nullptr;

			/// \note Unregister all engine listeners before cleaning up subsystems
			mEngineListeners.clear();

			result = result | _cleanUpSubsystems();

			CPoolAllocatorsRegistry::ClearAllAllocators();

#if TDE2_EDITORS_ENABLED
			U32 livingObjectsCount = CMemoryProfiler::Get()->GetLiveObjectsCount();
			CMemoryProfiler::Get()->DumpInfo();
			TDE2_ASSERT(!livingObjectsCount);
#endif
		}

		LOG_MESSAGE("[Engine Core] >>>=================================================");
		LOG_MESSAGE("[Engine Core] The engine's core was successfully finalized");
		LOG_MESSAGE("[Engine Core] <<<=================================================\n");

		return RC_OK;
	}

#if TDE2_EDITORS_ENABLED

	static void InitEditorCamera(IWindowSystem* pWindowSystem, ISceneManager* pSceneManager, TPtr<IWorld> pWorld)
	{
		TDE2_PROFILER_SCOPE("InitEditorCamera");

		if (auto pMainScene = pSceneManager->GetScene(MainScene).Get())
		{
			CEntity* pEditorCamera = pMainScene->CreateEditorCamera(static_cast<F32>(pWindowSystem->GetWidth() / pWindowSystem->GetHeight()), 0.5f * CMathConstants::Pi);

			if (auto pCamerasContextEntity = pWorld->FindEntity(pWorld->FindEntityWithUniqueComponent<CCamerasContextComponent>()))
			{
				if (auto pCamerasContext = pCamerasContextEntity->GetComponent<CCamerasContextComponent>())
				{
					if (TEntityId::Invalid == pCamerasContext->GetActiveCameraEntityId())
					{
						pCamerasContext->SetActiveCameraEntity(pEditorCamera->GetId());
					}
				}
			}
		}
	}

#endif


	E_RESULT_CODE CEngineCore::Run()
	{
		TDE2_PROFILER_SCOPE("CEngineCore::Run");

		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		//std::lock_guard<std::mutex> lock(mMutex);

		E_RESULT_CODE result = RC_OK;

		if (ISceneManager* pSceneManager = _getSubsystemAs<ISceneManager>(EST_SCENE_MANAGER))
		{
			mpWorldInstance = pSceneManager->GetWorld();
		}

		TDE2_ASSERT(mpWorldInstance);

#if TDE2_EDITORS_ENABLED
		IEditorsManager* pEditorsManager = _getSubsystemAs<IEditorsManager>(EST_EDITORS_MANAGER);
		TDE2_ASSERT(pEditorsManager);

		PANIC_ON_FAILURE(pEditorsManager->SetWorldInstance(mpWorldInstance));
#endif

		if (result != RC_OK)
		{
			return result;
		}

		/// \note Try to get a pointer to IImGUIContext implementation
		mpImGUIContext = _getSubsystemAs<IImGUIContext>(EST_IMGUI_CONTEXT);

		IWindowSystem* pWindowSystem = _getSubsystemAs<IWindowSystem>(EST_WINDOW);

		if (IPluginManager* pPluginManager = _getSubsystemAs<IPluginManager>(EST_PLUGIN_MANAGER))
		{
			pPluginManager->RegisterECSComponents(mpWorldInstance);
		}

		/// \note we can proceed if the window wasn't initialized properly or some error has happened within user's code
		if (!pWindowSystem || (_onNotifyEngineListeners(EET_ONSTART) != RC_OK))
		{
			return RC_FAIL;
		}

#if TDE2_EDITORS_ENABLED
		InitEditorCamera(pWindowSystem, _getSubsystemAs<ISceneManager>(EST_SCENE_MANAGER), mpWorldInstance);
#endif

		mpInternalTimer = pWindowSystem->GetTimer();

		/// \todo This is not critical error, but for now I leave it as is
		if ((result = _registerBuiltinSystems(mpWorldInstance, pWindowSystem,
			_getSubsystemAs<IGraphicsContext>(EST_GRAPHICS_CONTEXT),
			_getSubsystemAs<IRenderer>(EST_RENDERER), 
			_getSubsystemAs<IEventManager>(EST_EVENT_MANAGER))) != RC_OK)
		{
			return result;
		}

		if (IPluginManager* pPluginManager = _getSubsystemAs<IPluginManager>(EST_PLUGIN_MANAGER))
		{
			pPluginManager->RegisterECSSystems(mpWorldInstance);
		}

		mpInternalTimer->Start();

		mpInputContext = _getSubsystemAs<IInputContext>(EST_INPUT_CONTEXT);
#if TDE2_EDITORS_ENABLED
		mpEditorsManager = _getSubsystemAs<IEditorsManager>(EST_EDITORS_MANAGER);
#endif

		LOG_MESSAGE("");
		LOG_MESSAGE("[Engine Core] >>>=================================================");
		LOG_MESSAGE("[Engine Core] The engine's core begins to execute the main loop...");
		LOG_MESSAGE("[Engine Core] <<<=================================================\n");

		TDE2_STOP_REPORT_PROFILE();

		if (IsProfileStartupEnabled())
		{
			TDE2_SAVE_REPORT_PROFILE("TDEngine_StartUp");
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

		IWindowSystem* pWindowSystem = _getSubsystemAs<IWindowSystem>(EST_WINDOW);

		if (!pWindowSystem)
		{
			return RC_FAIL;
		}

		LOG_MESSAGE("[Engine Core] A signal to quit was received");

		return pWindowSystem->Quit();
	}

	E_RESULT_CODE CEngineCore::RegisterSubsystem(TPtr<IEngineSubsystem> pSubsystem)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _registerSubsystemInternal(pSubsystem);
	}

	E_RESULT_CODE CEngineCore::UnregisterSubsystem(E_ENGINE_SUBSYSTEM_TYPE subsystemType)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _unregisterSubsystem(subsystemType);
	}

	TResult<TEngineListenerId> CEngineCore::RegisterListener(std::unique_ptr<IEngineListener> pListener)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (!pListener)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		/// prevent duplicating instances
		auto it = std::find_if(mEngineListeners.cbegin(), mEngineListeners.cend(), [&](auto& pCurrListener) { return pListener.get() == pCurrListener.get(); });
		if (it != mEngineListeners.cend())
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		pListener->SetEngineInstance(this); /// inject engine's instance into the listener

		mEngineListeners.push_back(std::move(pListener));

		LOG_MESSAGE("[Engine Core] A new listener was successfully registered");

		return Wrench::TOkValue<TEngineListenerId>(static_cast<TEngineListenerId>(mEngineListeners.size() - 1));
	}

	E_RESULT_CODE CEngineCore::UnregisterListener(TEngineListenerId listenerHandle)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		const size_t index = static_cast<size_t>(listenerHandle);

		if ((TEngineListenerId::Invalid == listenerHandle) || (index >= mEngineListeners.size()))
		{
			return RC_INVALID_ARGS;
		}
		
		mEngineListeners.erase(mEngineListeners.cbegin() + index);

		LOG_MESSAGE("[Engine Core] The new listener was successfully unregistered");

		return RC_OK;
	}

	TPtr<ILogger> CEngineCore::GetLogger() const
	{
#if defined(TDE2_DEBUG_MODE)
		return MainLogger;
#else
		return nullptr;
#endif
	}

	TPtr<ITimer> CEngineCore::GetTimer() const
	{
		//std::lock_guard<std::mutex> lock(mMutex);

		return mpInternalTimer;
	}

	TPtr<IWorld> CEngineCore::GetWorldInstance() const
	{
		//std::lock_guard<std::mutex> lock(mMutex);

		return mpWorldInstance;
	}

	TPtr<IEngineSubsystem> CEngineCore::_getSubsystem(E_ENGINE_SUBSYSTEM_TYPE type) const
	{
		return (EST_UNKNOWN == type) ? TPtr<IEngineSubsystem>(nullptr) : mSubsystems[type];
	}

	void CEngineCore::_onFrameUpdateCallback()
	{
#if TDE2_EDITORS_ENABLED
		CPerfProfiler::Get()->BeginFrame();
		CMemoryProfiler::Get()->BeginFrame();

	#ifdef TDE2_USE_WINPLATFORM
		OPTICK_FRAME("MainThread");
	#endif
#endif
		{
			TDE2_BUILTIN_SPEC_PROFILER_EVENT(E_SPECIAL_PROFILE_EVENT::UPDATE);

			if (mpInputContext)
			{
				mpInputContext->Update();
			}

			_onNotifyEngineListeners(EET_ONUPDATE);

			if (auto pGameModesManager = _getSubsystemAs<IGameModesManager>(EST_GAME_MODES_MANAGER))
			{
				pGameModesManager->Update(mpInternalTimer->GetDeltaTime());
			}

			if (IAudioContext* pAudioContext = _getSubsystemAs<IAudioContext>(EST_AUDIO_CONTEXT))
			{
				pAudioContext->Update();
			}

			if (IRenderer* pRenderer = _getSubsystemAs<IRenderer>(EST_RENDERER))
			{
				pRenderer->GetFramePacketsStorage()->IncrementGameLogicFrameCounter();
			}
		}

		if (IRenderer* pRenderer = _getSubsystemAs<IRenderer>(EST_RENDERER))
		{
			pRenderer->Draw(mpInternalTimer->GetCurrTime(), mpInternalTimer->GetDeltaTime());
		}

		if (IJobManager* pJobManager = _getSubsystemAs<IJobManager>(EST_JOB_MANAGER))
		{
			pJobManager->ProcessMainThreadQueue();
		}

#if TDE2_EDITORS_ENABLED
		CPerfProfiler::Get()->EndFrame();
		CMemoryProfiler::Get()->EndFrame();
#endif

		++TFrameCounter::mGlobalFrameNumber;

		FrameMark;
	}

	E_RESULT_CODE CEngineCore::_onNotifyEngineListeners(E_ENGINE_EVENT_TYPE eventType)
	{
		TDE2_PROFILER_SCOPE("NotifyEngineListeners");

		if (mEngineListeners.empty())
		{
			return RC_OK;
		}

		E_RESULT_CODE result = RC_OK;

		F32 dt = 0.0f;

		for (auto&& pListener : mEngineListeners)
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

						mpWorldInstance->Update(dt);

						if (mpImGUIContext)
						{
							mpImGUIContext->BeginFrame(dt);
						}

#if TDE2_EDITORS_ENABLED
						if (mpEditorsManager)
						{
							mpEditorsManager->Update(dt);
						}

						mpWorldInstance->DebugOutput(mpDebugUtility, dt);
#endif

						mpWorldInstance->SyncSystemsExecution();

						/// \note The internal callback will be invoked when the execution process will go out of the scope
						defer([this]()
						{
							if (mpImGUIContext)
							{
								mpImGUIContext->EndFrame();
							}
						});

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

	E_RESULT_CODE CEngineCore::_registerBuiltinSystems(TPtr<IWorld> pWorldInstance, IWindowSystem* pWindowSystem, IGraphicsContext* pGraphicsContext,
													   IRenderer* pRenderer, IEventManager* pEventManager)
	{
		TDE2_PROFILER_SCOPE("_registerBuiltinSystems");

		IGraphicsObjectManager* pGraphicsObjectManager = pGraphicsContext->GetGraphicsObjectManager();
		IResourceManager* pResourceManager = _getSubsystemAs<IResourceManager>(EST_RESOURCE_MANAGER);

		E_RESULT_CODE result = RC_OK;

		if (auto debugUtilityResult = pGraphicsObjectManager->CreateDebugUtility(pResourceManager, pRenderer))
		{
			mpDebugUtility = debugUtilityResult.Get();
		}

		ISystem* p2dPhysics = nullptr;
		ISystem* p3dPhysics = nullptr;
		ISystem* pCameraSystem = nullptr;

		std::vector<ISystem*> builtinSystems
		{
			CreateTransformSystem(pGraphicsContext, result),
			CreateBoundsUpdatingSystem(pResourceManager, mpDebugUtility, _getSubsystemAs<ISceneManager>(EST_SCENE_MANAGER), result),
			CreateSpriteRendererSystem(TPtr<IAllocator>(CreateLinearAllocator(5 * SpriteInstanceDataBufferSize, result)),
									   pRenderer, pGraphicsObjectManager, result),
			(pCameraSystem = CreateCameraSystem(pWindowSystem, pGraphicsContext, pRenderer, result)),
			CreateLODMeshSwitchSystem(result),
			CreateWeatherSystem({ pResourceManager, pGraphicsContext, _getSubsystemAs<IJobManager>(EST_JOB_MANAGER) }, result),
			CreateStaticMeshRendererSystem(pRenderer, pGraphicsObjectManager, result),
			CreateAnimationSystem(pResourceManager, pEventManager, result),
			CreateMeshAnimatorUpdatingSystem(pResourceManager, result),
			CreateSkinnedMeshRendererSystem(pRenderer, pGraphicsObjectManager, result),
			CreateLightingSystem(pRenderer, pGraphicsObjectManager, mpInternalTimer, result),
			CProjectSettings::Get()->mGraphicsSettings.mIsGPUParticlesSimulationEnabled ? 
				CreateParticlesGPUSimulationSystem(pRenderer, pGraphicsObjectManager, result) :
				CreateParticlesSimulationSystem(pRenderer, pGraphicsObjectManager, result),
			
			CreateAsyncSystemsGroup(pEventManager, /// \note Systems within the group is executed sequentially, but the group itself runs at worker thread
				{
					CreateUIEventsSystem(_getSubsystemAs<IInputContext>(EST_INPUT_CONTEXT), mpImGUIContext, result),
					CreateUIElementsProcessSystem(pGraphicsContext, pResourceManager,_getSubsystemAs<ISceneManager>(EST_SCENE_MANAGER), result),
					CreateUIElementsRenderSystem(pRenderer, pGraphicsObjectManager, result),
				}, result),

#if TDE2_EDITORS_ENABLED
			CreateObjectsSelectionSystem(pRenderer, pGraphicsObjectManager, result),
#endif
			(p2dPhysics = CreatePhysics2DSystem(pEventManager, result)),
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

		if (auto pBaseCameraSystem = dynamic_cast<ICameraSystem*>(pCameraSystem))
		{
			pBaseCameraSystem->SetDebugUtility(mpDebugUtility);
		}

		auto pRaycastContextInstance = CreateBaseRaycastContext(dynamic_cast<CPhysics2DSystem*>(p2dPhysics), 
																nullptr, 
																result);

		if ((result != RC_OK) || (result = pWorldInstance->RegisterRaycastContext(TPtr<IRaycastContext>(pRaycastContextInstance))) != RC_OK)
		{
			return result;
		}
	
		// \note Send event that a new world was created
		{
			TOnNewWorldInstanceCreated onNewWorldInstanceCreated;
			onNewWorldInstanceCreated.mpWorldInstance = pWorldInstance;

			pEventManager->Notify(&onNewWorldInstanceCreated);
		}

		return RC_OK;
	}

	E_RESULT_CODE CEngineCore::_registerSubsystemInternal(TPtr<IEngineSubsystem> pSubsystem)
	{
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

		LOG_MESSAGE(std::string("[Engine Core] A new subsystem was successfully registered: ").append(Meta::EnumTrait<E_ENGINE_SUBSYSTEM_TYPE>::ToString(pSubsystem->GetType())));

		return RC_OK;
	}

	E_RESULT_CODE CEngineCore::_unregisterSubsystem(E_ENGINE_SUBSYSTEM_TYPE subsystemType)
	{
		TPtr<IEngineSubsystem> pEngineSubsystem = mSubsystems[subsystemType];

		if (!pEngineSubsystem)
		{
			return RC_OK;
		}

		mSubsystems[subsystemType] = nullptr;

		const static U16 statusStringLength = 64;

		std::string subsystemName = std::move(std::string("[Engine Core] ").append(Meta::EnumTrait<E_ENGINE_SUBSYSTEM_TYPE>::ToString(subsystemType)));

		MainLogger->LogStatus(subsystemName, "OK", '.', static_cast<U16>(statusStringLength - subsystemName.size()));

		return RC_OK;
	}

	E_RESULT_CODE CEngineCore::_cleanUpSubsystems()
	{
		E_RESULT_CODE result = RC_OK;

		LOG_MESSAGE("[Engine Core] Clean up the subsystems registry...");

		auto pPluginManager = _getSubsystem(E_ENGINE_SUBSYSTEM_TYPE::EST_PLUGIN_MANAGER); /// \note The plugin manager should be released at last moment

		{
			auto pGraphicsContext = _getSubsystem(E_ENGINE_SUBSYSTEM_TYPE::EST_GRAPHICS_CONTEXT);

			/// frees memory from all subsystems
			for (auto&& pCurrSubsystem : mSubsystems)
			{
				if (!pCurrSubsystem)
				{
					continue;
				}

				result = _unregisterSubsystem(pCurrSubsystem->GetType());
			}
		}

		return result;
	}
	

	IEngineCore* CreateEngineCore(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEngineCore, CEngineCore, result);
	}
}