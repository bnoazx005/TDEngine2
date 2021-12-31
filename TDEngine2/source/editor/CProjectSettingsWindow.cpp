#include "../../include/editor/CProjectSettingsWindow.h"
#include "../../include/core/IImGUIContext.h"
#include "../../include/core/IEventManager.h"
#include "../../include/ecs/CWorld.h"
#include "../../include/ecs/ISystem.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	CProjectSettingsWindow::CProjectSettingsWindow() :
		CBaseEditorWindow()
	{
	}

	E_RESULT_CODE CProjectSettingsWindow::Init(TPtr<IEventManager> pEventManager)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!pEventManager)
		{
			return RC_INVALID_ARGS;
		}

		pEventManager->Subscribe(TOnNewWorldInstanceCreated::GetTypeId(), this);

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CProjectSettingsWindow::OnEvent(const TBaseEvent* pEvent)
	{
		if (TOnNewWorldInstanceCreated::GetTypeId() != pEvent->GetEventType())
		{
			return RC_OK;
		}

		if (auto&& pWorldEvent = dynamic_cast<const TOnNewWorldInstanceCreated*>(pEvent))
		{
			mpWorld = pWorldEvent->mpWorldInstance;
		}

		return RC_OK;
	}

	TEventListenerId CProjectSettingsWindow::GetListenerId() const
	{
		return TEventListenerId(GetTypeId());
	}


	static void ShowSystemsListSubwindow(IImGUIContext& imguiContext, TPtr<IWorld> pWorld)
	{
		if (!imguiContext.BeginChildWindow("##SystemsListWindow", TVector2(imguiContext.GetWindowWidth() * 0.95f, imguiContext.GetWindowHeight() * 0.8f)))
		{
			return;
		}

		pWorld->ForEachSystem([&imguiContext, pWorld](TSystemId systemId, const ISystem* pSystem)
		{
			const bool isSelected = pSystem->IsActive(); 
			bool isNewState = isSelected;

			imguiContext.BeginHorizontal();
			imguiContext.Label(pSystem->GetName());
			imguiContext.Checkbox(Wrench::StringUtils::Format("##{0}", std::to_string(static_cast<U32>(systemId))), isNewState);
			imguiContext.EndHorizontal();

			if (isNewState != isSelected)
			{
				if (isNewState)
				{
					pWorld->ActivateSystem(systemId);
				}
				else
				{
					pWorld->DeactivateSystem(systemId);
				}
			}
		});

		imguiContext.EndChildWindow();
	}


	void CProjectSettingsWindow::_onDraw()
	{
		bool isEnabled = mIsVisible;

		static const IImGUIContext::TWindowParams params
		{
			ZeroVector2,
			TVector2(400.0f, 400.0f),
			TVector2(800.0f, 800.0f),
		};

		static const std::tuple<std::string, std::function<void(IImGUIContext&)>> projectSettingsSections[1]
		{
			{ "Systems List", [this](auto& imguiContext) { ShowSystemsListSubwindow(imguiContext, mpWorld); } },
		};

		if (mpImGUIContext->BeginWindow("Project Settings", isEnabled, params))
		{
			for (auto&& currEntity : projectSettingsSections)
			{
				if (mpImGUIContext->CollapsingHeader(std::get<std::string>(currEntity), false, false))
				{
					std::get<1>(currEntity)(*mpImGUIContext);
				}
			}
		}

		mpImGUIContext->EndWindow();

		mIsVisible = isEnabled;
	}


	TDE2_API IEditorWindow* CreateProjectSettingsWindow(TPtr<IEventManager> pEventManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEditorWindow, CProjectSettingsWindow, result, pEventManager);
	}
}

#endif