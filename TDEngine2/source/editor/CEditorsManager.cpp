#include "../../include/editor/CEditorsManager.h"
#include "../../include/editor/IEditorWindow.h"
#include "../../include/core/IImGUIContext.h"
#include "../../include/core/IInputContext.h"
#include "../../include/core/IWindowSystem.h"
#include "../../include/editor/CPerfProfiler.h"
#include "../../include/editor/ecs/CEditorCameraControlSystem.h"
#include "../../include/ecs/CWorld.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/editor/ISelectionManager.h"
#include "../../include/core/IEventManager.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/graphics/CBaseCamera.h"
#include "stringUtils.hpp"
#include <algorithm>


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	const std::unordered_map<E_EDITOR_TYPE, std::string> CEditorsManager::mEditorNamesMap
	{
		{ E_EDITOR_TYPE::TIME_PROFILER, "Time Profiler" },
		{ E_EDITOR_TYPE::MEMORY_PROFILER, "Memory Profiler" },
		{ E_EDITOR_TYPE::LEVEL_EDITOR, "Level Editor" },
		{ E_EDITOR_TYPE::DEV_CONSOLE, "Console" },
		{ E_EDITOR_TYPE::RENDER_TARGET_VIEWER, "RTs Viewer" },
		{ E_EDITOR_TYPE::PROJECT_SETTINGS_EDITOR, "Project Settings" },
	};


	CEditorsManager::CEditorsManager():
		CBaseObject(), mIsVisible(false)
	{
	}

	E_RESULT_CODE CEditorsManager::Init(TPtr<IInputContext> pInputContext, TPtr<IImGUIContext> pImGUIContext, TPtr<IEventManager> pEventManager, TPtr<IWorld> pWorld)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!pImGUIContext || !pInputContext)
		{
			return RC_INVALID_ARGS;
		}

		mpInputContext = DynamicPtrCast<IDesktopInputContext>(pInputContext);
		mpImGUIContext = pImGUIContext;
		mpWorld        = pWorld;
		mpEventManager = pEventManager;

		mpEventManager->Subscribe(TOnNewWorldInstanceCreated::GetTypeId(), this);
		
		mIsVisible = false;

		if (!mpInputContext)
		{
			return RC_INVALID_ARGS;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CEditorsManager::_onFreeInternal()
	{
		E_RESULT_CODE result = RC_OK;

		for (auto& currEditorEntry : mRegisteredEditors)
		{
			IEditorWindow* pCurrEditorWindow = std::get<IEditorWindow*>(currEditorEntry);
			if (!pCurrEditorWindow)
			{
				continue;
			}

			result = result | pCurrEditorWindow->Free();
		}

		return result;
	}

	E_RESULT_CODE CEditorsManager::RegisterEditor(const std::string& commandName, IEditorWindow* pEditorWindow, bool isSeparate)
	{
		if (commandName.empty() || !pEditorWindow)
		{
			return RC_INVALID_ARGS;
		}

		if (std::find_if(mRegisteredEditors.cbegin(), mRegisteredEditors.cend(), [&commandName](auto&& entry)
		{
			return std::get<std::string>(entry) == commandName;
		}) != mRegisteredEditors.cend())
		{
			return RC_FAIL;
		}

		mRegisteredEditors.emplace_back(commandName, pEditorWindow, isSeparate);

		return RC_OK;
	}


	static void SetEditorCameraActive(TPtr<IWorld> pWorld, bool enabled)
	{
		CEntity* pCameraEntity = pWorld->FindEntity(pWorld->FindEntitiesWithComponents<CEditorCamera>().front());
		if (!pCameraEntity)
		{
			return;
		}

		if (auto pCamerasContextEntity = pWorld->FindEntity(pWorld->FindEntityWithUniqueComponent<CCamerasContextComponent>()))
		{
			if (auto pCamerasContext = pCamerasContextEntity->GetComponent<CCamerasContextComponent>())
			{
				if (enabled)
				{
					pCamerasContext->SetActiveCameraEntity(pCameraEntity->GetId());
				}
				else
				{
					pCamerasContext->RestorePreviousCameraEntity();
				}
			}
		}
	}


	E_RESULT_CODE CEditorsManager::Update(F32 dt)
	{
		TDE2_PROFILER_SCOPE("EditorsManager::Update");

		if (mpInputContext->IsKeyPressed(E_KEYCODES::KC_TILDE))
		{
			mIsVisible = !mIsVisible;

			TOnEditorModeEnabled  onEditorEnabledEvent;
			TOnEditorModeDisabled onEditorDisabledEvent;

			TBaseEvent* pEvent = mIsVisible ? dynamic_cast<TBaseEvent*>(&onEditorEnabledEvent) : dynamic_cast<TBaseEvent*>(&onEditorDisabledEvent);
			mpEventManager->Notify(pEvent);

			SetEditorCameraActive(mpWorld, mIsVisible);
		}

		return _showEditorWindows(dt);
	}

	E_RESULT_CODE CEditorsManager::SetWorldInstance(TPtr<IWorld> pWorld)
	{
		if (!pWorld)
		{
			return RC_INVALID_ARGS;
		}

		mpWorld = pWorld;

		E_RESULT_CODE result = RC_OK;

		if (mEditorCameraControlSystemId == TSystemId::Invalid)
		{
			auto registerResult = mpWorld->RegisterSystem(CreateEditorCameraControlSystem(mpInputContext.Get(), this, result), E_SYSTEM_PRIORITY::SP_NORMAL_PRIORITY);
			if (registerResult.HasError())
			{
				return result;
			}

			mEditorCameraControlSystemId = registerResult.Get();
		}

		return result;
	}

	E_RESULT_CODE CEditorsManager::SetSelectionManager(TPtr<ISelectionManager> pSelectionManager)
	{
		if (!pSelectionManager)
		{
			return RC_INVALID_ARGS;
		}

		mpSelectionManager = pSelectionManager;

		mpEventManager->Subscribe(TOnEditorModeEnabled::GetTypeId(), mpSelectionManager.Get());
		mpEventManager->Subscribe(TOnEditorModeDisabled::GetTypeId(), mpSelectionManager.Get());
		mpEventManager->Subscribe(TOnWindowResized::GetTypeId(), mpSelectionManager.Get());

		return RC_OK;
	}

	E_ENGINE_SUBSYSTEM_TYPE CEditorsManager::GetType() const
	{
		return EST_EDITORS_MANAGER;
	}

	bool CEditorsManager::IsEditorModeEnabled() const
	{
		const std::string mLevelEditorName = mEditorNamesMap.at(E_EDITOR_TYPE::LEVEL_EDITOR);

		auto editorIter = std::find_if(mRegisteredEditors.cbegin(), mRegisteredEditors.cend(), [&mLevelEditorName](const auto& entry)
		{
			return std::get<IEditorWindow*>(entry) && (std::get<std::string>(entry) == mLevelEditorName);
		});

		return (editorIter != mRegisteredEditors.cend()) ? std::get<IEditorWindow*>(*editorIter)->IsVisible() : false;
	}

	TPtr<ISelectionManager> CEditorsManager::GetSelectionManager() const
	{
		return mpSelectionManager;
	}

	E_RESULT_CODE CEditorsManager::OnEvent(const TBaseEvent* pEvent)
	{
		if (pEvent->GetEventType() != TOnNewWorldInstanceCreated::GetTypeId())
		{
			return RC_OK;
		}

		if (auto pOnWorldInstanceCreated = dynamic_cast<const TOnNewWorldInstanceCreated*>(pEvent))
		{
			PANIC_ON_FAILURE(mpSelectionManager->SetWorldInstance(pOnWorldInstanceCreated->mpWorldInstance));
		}

		return RC_OK;
	}

	TEventListenerId CEditorsManager::GetListenerId() const
	{
		return TEventListenerId(GetTypeId());
	}

	TPtr<IWorld> CEditorsManager::GetWorldInstance() const
	{
		return mpWorld;
	}

	E_RESULT_CODE CEditorsManager::_showEditorWindows(F32 dt)
	{
		if (!mIsVisible)
		{
			return RC_OK;
		}

		const static TVector2 buttonSizes { 150.0f, 20.0f };
		
		static const IImGUIContext::TWindowParams params
		{
			ZeroVector2,
			TVector2(166.0f, 300.0f),
			TVector2(166.0f, 300.0f),
		};

		{
			if (mpImGUIContext->BeginWindow("Development Menu", mIsVisible, params))
			{
				std::string currCommandName;
				IEditorWindow* pCurrEditorWindow = nullptr;
				bool isSeparate = false;

				// \todo Draw all buttons here, next step is to add sub-menus based on states
				for (auto& pCurrEditorWindowEntry : mRegisteredEditors)
				{
					std::tie(currCommandName, pCurrEditorWindow, isSeparate) = pCurrEditorWindowEntry;

					if (isSeparate)
					{
						continue;
					}

					bool isEditorEnabled = pCurrEditorWindow->IsVisible();

					mpImGUIContext->Button(Wrench::StringUtils::Format("{0} {1}", isEditorEnabled ? "Hide " : "Show ", currCommandName),
						buttonSizes, [isEditorEnabled, pCurrEditorWindow]()
					{
						pCurrEditorWindow->SetVisible(!isEditorEnabled);
					});
				}
			}

			mpImGUIContext->EndWindow();
		}

		for (auto& currEditorEntry : mRegisteredEditors)
		{
			IEditorWindow* pCurrEditorWindow = std::get<IEditorWindow*>(currEditorEntry);
			if (!pCurrEditorWindow)
			{
				continue;
			}

			pCurrEditorWindow->Draw(mpImGUIContext.Get(), dt);
		}

		return RC_OK;
	}


	TDE2_API IEditorsManager* CreateEditorsManager(TPtr<IInputContext> pInputContext, TPtr<IImGUIContext> pImGUIContext, TPtr<IEventManager> pEventManager, TPtr<IWorld> pWorld, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEditorsManager, CEditorsManager, result, pInputContext, pImGUIContext, pEventManager, pWorld);
	}
}

#endif