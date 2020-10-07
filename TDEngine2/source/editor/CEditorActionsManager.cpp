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

		mpActionsHistory.push_back(pAction);

		return RC_OK;
	}

	TResult<IEditorAction*> CEditorActionsManager::PopAction()
	{
		if (mpActionsHistory.empty())
		{
			return TErrorValue<E_RESULT_CODE>(RC_FAIL);
		}

		IEditorAction* pAction = mpActionsHistory.back();
		mpActionsHistory.pop_back();

		return TOkValue<IEditorAction*>(pAction);
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
		CEditorActionsManager* pManagerInstance = new (std::nothrow) CEditorActionsManager();

		if (!pManagerInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pManagerInstance->Init();

		if (result != RC_OK)
		{
			delete pManagerInstance;

			pManagerInstance = nullptr;
		}

		return dynamic_cast<IEditorActionsHistory*>(pManagerInstance);
	}
}

#endif