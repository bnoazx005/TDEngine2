#include "../include/CMaterialEditorWindow.h"


namespace TDEngine2
{
	CMaterialEditorWindow::CMaterialEditorWindow() :
		CBaseEditorWindow()
	{
	}

	E_RESULT_CODE CMaterialEditorWindow::Init(IResourceManager* pResourceManager, IInputContext* pInputContext)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!pResourceManager || !pResourceManager)
		{
			return RC_INVALID_ARGS;
		}

		E_RESULT_CODE result = RC_OK;

		mpResourceManager = pResourceManager;
		mpInputContext = dynamic_cast<IDesktopInputContext*>(pInputContext);

		mpEditorHistory = CreateEditorActionsManager(result);
		if (RC_OK != result)
		{
			return result;
		}

		mCurrMaterialId = TResourceId::Invalid;
		mpCurrMaterial = nullptr;

		mIsInitialized = true;
		mIsVisible = true;

		return RC_OK;
	}

	void CMaterialEditorWindow::SetMaterialResourceHandle(TResourceId handle)
	{
		mCurrMaterialId = handle;
		mpCurrMaterial = mpResourceManager->GetResource<IMaterial>(mCurrMaterialId);
	}

	E_RESULT_CODE CMaterialEditorWindow::ExecuteUndoAction()
	{
		if (!mpEditorHistory)
		{
			return RC_FAIL;
		}

		if (auto actionResult = mpEditorHistory->PopAction())
		{
			return actionResult.Get()->Restore();
		}

		return RC_OK;
	}

	E_RESULT_CODE CMaterialEditorWindow::ExecuteRedoAction()
	{
		if (!mpEditorHistory)
		{
			return RC_FAIL;
		}

		if (auto actionResult = mpEditorHistory->PopAction())
		{
			return actionResult.Get()->Execute();
		}

		return RC_OK;
	}

	void CMaterialEditorWindow::_onDraw()
	{
		bool isEnabled = mIsVisible;

		static const IImGUIContext::TWindowParams params
		{
			ZeroVector2,
			TVector2(350.0f, 200.0f),
			TVector2(1000.0f, 550.0f),
		};

		if (!mpCurrMaterial)
		{
			return;
		}

		if (mpImGUIContext->BeginWindow("Material Editor", isEnabled, params))
		{
			
		}

		mpImGUIContext->EndWindow();

		E_RESULT_CODE result = RC_OK;

		// \note process shortcuts		
		if (mpInputContext->IsKey(E_KEYCODES::KC_LCONTROL))
		{
			if (mpInputContext->IsKeyPressed(E_KEYCODES::KC_Z)) // \note Ctrl+Z
			{
				mpEditorHistory->ExecuteUndo();
				mpEditorHistory->Dump();
			}

			if (mpInputContext->IsKeyPressed(E_KEYCODES::KC_Y)) // \note Ctrl+Y
			{
				mpEditorHistory->ExecuteRedo();
				mpEditorHistory->Dump();
			}
		}

		mIsVisible = isEnabled;
	}


#define STRINGIFY_COMMAND(...) __VA_ARGS__

#define MAKE_COMMAND(historyOwner, command, newValue, oldValue)									\
	do                                                                                          \
	{																							\
		E_RESULT_CODE result = RC_OK;                                                           \
                                                                                                \
		IEditorAction* pAction = CreateCommandAction(                                           \
			[this, value = newValue] { command(value); },                                       \
			[this, value = oldValue] { command(value); },                                       \
			result);                                                                            \
                                                                                                \
		historyOwner->PushAndExecuteAction(pAction);                                            \
	}                                                                                           \
	while (0)


#define MAKE_COMMAND_CAPTURE(historyOwner, command, capture, newValue, oldValue)				\
	do                                                                                          \
	{																							\
		E_RESULT_CODE result = RC_OK;                                                           \
                                                                                                \
		IEditorAction* pAction = CreateCommandAction(                                           \
			[capture, this, value = newValue] { command(value); },                              \
			[capture, this, value = oldValue] { command(value); },                              \
			result);                                                                            \
                                                                                                \
		historyOwner->PushAndExecuteAction(pAction);                                            \
	}                                                                                           \
	while (0)


	TDE2_API IEditorWindow* CreateMaterialEditorWindow(IResourceManager* pResourceManager, IInputContext* pInputContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEditorWindow, CMaterialEditorWindow, result, pResourceManager, pInputContext);
	}
}