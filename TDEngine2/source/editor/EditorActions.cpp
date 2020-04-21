#include "../../include/editor/EditorActions.h"
#include "../../include/editor/CLevelEditorWindow.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	CUndoEditorAction::CUndoEditorAction() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CUndoEditorAction::Init(CLevelEditorWindow* pEditorWindow)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!pEditorWindow)
		{
			return RC_INVALID_ARGS;
		}

		mpEditorWindow = pEditorWindow;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CUndoEditorAction::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;
		delete this;

		return RC_OK;
	}

	E_RESULT_CODE CUndoEditorAction::Execute()
	{
		return mpEditorWindow->ExecuteUndoAction();
	}

	E_RESULT_CODE CUndoEditorAction::Restore()
	{
		return mpEditorWindow->ExecuteRedoAction();
	}

	std::string CUndoEditorAction::ToString() const
	{
		return "CUndoEditorAction";
	}


	TDE2_API IEditorAction* CreateUndoAction(CLevelEditorWindow* pEditorWindow, E_RESULT_CODE& result)
	{
		CUndoEditorAction* pActionInstance = new (std::nothrow) CUndoEditorAction();

		if (!pActionInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pActionInstance->Init(pEditorWindow);

		if (result != RC_OK)
		{
			delete pActionInstance;

			pActionInstance = nullptr;
		}

		return dynamic_cast<IEditorAction*>(pActionInstance);
	}


	/*!
		\note CRedoEditorAction's definition
	*/

	CRedoEditorAction::CRedoEditorAction() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CRedoEditorAction::Init(CLevelEditorWindow* pEditorWindow)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!pEditorWindow)
		{
			return RC_INVALID_ARGS;
		}

		mpEditorWindow = pEditorWindow;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CRedoEditorAction::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;
		delete this;

		return RC_OK;
	}

	E_RESULT_CODE CRedoEditorAction::Execute()
	{
		return mpEditorWindow->ExecuteRedoAction();
	}

	E_RESULT_CODE CRedoEditorAction::Restore()
	{
		return mpEditorWindow->ExecuteUndoAction();
	}

	std::string CRedoEditorAction::ToString() const
	{
		return "CRedoEditorAction";
	}


	TDE2_API IEditorAction* CreateRedoAction(CLevelEditorWindow* pEditorWindow, E_RESULT_CODE& result)
	{
		CRedoEditorAction* pActionInstance = new (std::nothrow) CRedoEditorAction();

		if (!pActionInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pActionInstance->Init(pEditorWindow);

		if (result != RC_OK)
		{
			delete pActionInstance;

			pActionInstance = nullptr;
		}

		return dynamic_cast<IEditorAction*>(pActionInstance);
	}
}

#endif