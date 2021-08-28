#include "../include/CTrackSheetEditor.h"


namespace TDEngine2
{
	CTrackSheetEditor::CTrackSheetEditor() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CTrackSheetEditor::Init()
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CTrackSheetEditor::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		--mRefCounter;

		if (!mRefCounter)
		{
			mIsInitialized = false;
			delete this;
		}

		return RC_OK;
	}


	TDE2_API CTrackSheetEditor* CreateTrackSheetEditor(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(CTrackSheetEditor, CTrackSheetEditor, result);
	}
}