#include "./../../include/editor/CEditorsManager.h"
#include "./../../include/editor/IEditorWindow.h"
#include "./../../include/core/IImGUIContext.h"
#include "./../../include/core/IInputContext.h"
#include "./../../include/editor/CPerfProfiler.h"
#include <algorithm>


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	CEditorsManager::CEditorsManager():
		CBaseObject(), mIsVisible(false)
	{
	}

	E_RESULT_CODE CEditorsManager::Init(IInputContext* pInputContext, IImGUIContext* pImGUIContext)
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

	E_RESULT_CODE CEditorsManager::Update()
	{
		TDE2_PROFILER_SCOPE("EditorsManager::Update");

		if (mpInputContext->IsKeyPressed(E_KEYCODES::KC_TILDE))
		{
			mIsVisible = !mIsVisible;
		}

		return _showEditorWindows();
	}

	E_ENGINE_SUBSYSTEM_TYPE CEditorsManager::GetType() const
	{
		return EST_EDITORS_MANAGER;
	}

	E_RESULT_CODE CEditorsManager::_showEditorWindows()
	{
		if (!mIsVisible)
		{
			return RC_OK;
		}

		const static TVector2 buttonSizes { 150.0f, 20.0f };
		
		{
			if (mpImGUIContext->BeginWindow("Development Menu", mIsVisible))
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

			pCurrEditorWindow->Draw(mpImGUIContext);
		}

		return RC_OK;
	}


	TDE2_API IEditorsManager* CreateEditorsManager(IInputContext* pInputContext, IImGUIContext* pImGUIContext, E_RESULT_CODE& result)
	{
		CEditorsManager* pEditorsManagerInstance = new (std::nothrow) CEditorsManager();

		if (!pEditorsManagerInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pEditorsManagerInstance->Init(pInputContext, pImGUIContext);

		if (result != RC_OK)
		{
			delete pEditorsManagerInstance;

			pEditorsManagerInstance = nullptr;
		}

		return dynamic_cast<IEditorsManager*>(pEditorsManagerInstance);
	}
}

#endif