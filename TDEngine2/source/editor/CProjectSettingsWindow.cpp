#include "../../include/editor/CProjectSettingsWindow.h"
#include "../../include/core/IImGUIContext.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	CProjectSettingsWindow::CProjectSettingsWindow() :
		CBaseEditorWindow()
	{
	}

	E_RESULT_CODE CProjectSettingsWindow::Init()
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		mIsInitialized = true;

		return RC_OK;
	}


	static void ShowSystemsListSubwindow(IImGUIContext& imguiContext)
	{
		if (!imguiContext.BeginChildWindow("##SystemsListWindow", TVector2(imguiContext.GetWindowWidth() * 0.8f, imguiContext.GetWindowHeight() * 0.8f)))
		{
			return;
		}

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
			{ "Systems List", ShowSystemsListSubwindow },
		};

		if (mpImGUIContext->BeginWindow("Project Settings", isEnabled, params))
		{
			for (auto&& currEntity : projectSettingsSections)
			{
				if (mpImGUIContext->CollapsingHeader(std::get<std::string>(currEntity), true, false))
				{
					std::get<1>(currEntity)(*mpImGUIContext);
				}
			}
		}

		mpImGUIContext->EndWindow();

		mIsVisible = isEnabled;
	}


	TDE2_API IEditorWindow* CreateProjectSettingsWindow(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEditorWindow, CProjectSettingsWindow, result);
	}
}

#endif