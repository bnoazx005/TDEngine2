#include "../../include/editor/CEditorActionsManager.h"
#include "../../include/editor/EditorActions.h"
#include "../../include/utils/Utils.h"
#include "../../include/utils/CFileLogger.h"
#include "stringUtils.hpp"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	CEditorActionsManager::CEditorActionsManager():
		CBaseObject()
	{
	}

	E_RESULT_CODE CEditorActionsManager::Init()
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		mCurrActionPointer = 0;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CEditorActionsManager::PushAndExecuteAction(IEditorAction* pAction)
	{
		if (!pAction)
		{
			return RC_INVALID_ARGS;
		}

		mpActionsHistory.emplace(mpActionsHistory.begin() + mCurrActionPointer, pAction);
		++mCurrActionPointer;

		return pAction->Execute();
	}

	TResult<CScopedPtr<IEditorAction>> CEditorActionsManager::PopAction()
	{
		if (mpActionsHistory.empty())
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		CScopedPtr<IEditorAction> pAction = mpActionsHistory.back();
		mpActionsHistory.pop_back();

		return Wrench::TOkValue<CScopedPtr<IEditorAction>>(pAction);
	}

	E_RESULT_CODE CEditorActionsManager::ExecuteUndo()
	{
		if (mpActionsHistory.empty() || (static_cast<I32>(mCurrActionPointer) - 1 < 0))
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = mpActionsHistory[mCurrActionPointer - 1]->Restore();

		if (mCurrActionPointer >= 1)
		{
			--mCurrActionPointer;
		}

		return result;
	}

	E_RESULT_CODE CEditorActionsManager::ExecuteRedo()
	{
		if (mpActionsHistory.empty() || (mCurrActionPointer >= mpActionsHistory.size()))
		{
			return RC_FAIL;
		}

		++mCurrActionPointer;

		E_RESULT_CODE result = mpActionsHistory[mCurrActionPointer - 1]->Execute();
		
		return result;
	}

	void CEditorActionsManager::Dump() const
	{
		LOG_MESSAGE(Wrench::StringUtils::Format("[CEditorActionsManager] Current log size: {0}", mpActionsHistory.size()));

		for (auto iter = mpActionsHistory.rbegin(); iter != mpActionsHistory.rend(); ++iter)
		{
			LOG_MESSAGE(Wrench::StringUtils::Format("- {0}", (*iter)->ToString()));
		}
	}


	TDE2_API IEditorActionsHistory* CreateEditorActionsManager(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEditorActionsHistory, CEditorActionsManager, result);
	}
}

#endif