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

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CEditorActionsManager::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;
		delete this;

		return RC_OK;
	}

	E_RESULT_CODE CEditorActionsManager::PushAction(IEditorAction* pAction)
	{
		if (!pAction)
		{
			return RC_INVALID_ARGS;
		}

		mpActionsHistory.emplace_back(pAction);

		return RC_OK;
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