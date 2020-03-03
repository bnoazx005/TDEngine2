#include "./../../include/editor/CEditorsManager.h"
#include "./../../include/editor/IEditorWindow.h"
#include "./../../include/core/IImGUIContext.h"
#include "./../../include/core/IInputContext.h"
#include "./../../include/editor/CPerfProfiler.h"
#include "./../../include/editor/ecs/CEditorCameraControlSystem.h"
#include "./../../include/ecs/CWorld.h"
#include "./../../include/editor/ISelectionManager.h"
#include "./../../include/core/IEventManager.h"
#include <algorithm>


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	const std::unordered_map<E_EDITOR_TYPE, std::string> CEditorsManager::mEditorNamesMap
	{
		{ E_EDITOR_TYPE::PROFILER, "Profiler" },
		{ E_EDITOR_TYPE::LEVEL_EDITOR, "Level Editor" },
		{ E_EDITOR_TYPE::DEV_CONSOLE, "Console" },
	};


	CEditorsManager::CEditorsManager():
		CBaseObject(), mIsVisible(false)
	{
	}

	E_RESULT_CODE CEditorsManager::Init(IInputContext* pInputContext, IImGUIContext* pImGUIContext, IEventManager* pEventManager, IWorld* pWorld)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!pImGUIContext || !pInputContext)
		{
			return RC_INVALID_ARGS;
		}

		mpInputContext = dynamic_cast<IDesktopInputContext*>(pInputContext);
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

	E_RESULT_CODE CEditorsManager::Free()
	{
		if (!mIsInitialized)
		{
			return RC_OK;
		}

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

		mIsInitialized = false;
		delete this;

		return RC_OK;
	}

	E_RESULT_CODE CEditorsManager::RegisterEditor(const std::string& commandName, IEditorWindow* pEditorWindow)
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

		mRegisteredEditors.emplace_back(commandName, pEditorWindow);

		return RC_OK;
	}

	E_RESULT_CODE CEditorsManager::Update(F32 dt)
	{
		TDE2_PROFILER_SCOPE("EditorsManager::Update");

		if (mpInputContext->IsKeyPressed(E_KEYCODES::KC_TILDE))
		{
			mIsVisible = !mIsVisible;
		}

		return _showEditorWindows(dt);
	}

	E_RESULT_CODE CEditorsManager::SetWorldInstance(IWorld* pWorld)
	{
		if (!pWorld)
		{
			return RC_INVALID_ARGS;
		}

		mpWorld = pWorld;

		E_RESULT_CODE result = RC_OK;

		if (mEditorCameraControlSystemId == InvalidSystemId)
		{
			auto registerResult = mpWorld->RegisterSystem(CreateEditorCameraControlSystem(mpInputContext, this, result), E_SYSTEM_PRIORITY::SP_NORMAL_PRIORITY);
			if (registerResult.HasError())
			{
				return result;
			}

			mEditorCameraControlSystemId = registerResult.Get();
		}

		return result;
	}

	E_RESULT_CODE CEditorsManager::SetSelectionManager(ISelectionManager* pSelectionManager)
	{
		if (!pSelectionManager)
		{
			return RC_INVALID_ARGS;
		}

		mpSelectionManager = pSelectionManager;

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

	ISelectionManager* CEditorsManager::GetSelectionManager() const
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
		return GetTypeId();
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

				// \todo Draw all buttons here, next step is to add sub-menus based on states
				for (auto& pCurrEditorWindowEntry : mRegisteredEditors)
				{
					std::tie(currCommandName, pCurrEditorWindow) = pCurrEditorWindowEntry;

					bool isEditorEnabled = pCurrEditorWindow->IsVisible();

					mpImGUIContext->Button(CStringUtils::Format("{0} {1}", isEditorEnabled ? "Hide " : "Show ", currCommandName),
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

			pCurrEditorWindow->Draw(mpImGUIContext, dt);
		}

		return RC_OK;
	}


	TDE2_API IEditorsManager* CreateEditorsManager(IInputContext* pInputContext, IImGUIContext* pImGUIContext, IEventManager* pEventManager, IWorld* pWorld, E_RESULT_CODE& result)
	{
		CEditorsManager* pEditorsManagerInstance = new (std::nothrow) CEditorsManager();

		if (!pEditorsManagerInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pEditorsManagerInstance->Init(pInputContext, pImGUIContext, pEventManager, pWorld);

		if (result != RC_OK)
		{
			delete pEditorsManagerInstance;

			pEditorsManagerInstance = nullptr;
		}

		return dynamic_cast<IEditorsManager*>(pEditorsManagerInstance);
	}
}

#endif