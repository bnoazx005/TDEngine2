#include "../../include/core/CEventManager.h"
#include "../../include/core/Event.h"
#include <algorithm>


namespace TDEngine2
{
	CEventManager::CEventManager():
		CBaseObject()
	{
	}

	E_RESULT_CODE CEventManager::Init() 
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CEventManager::Subscribe(TypeId eventType, IEventHandler* pEventListener)
	{
		if (!pEventListener)
		{
			return RC_INVALID_ARGS;
		}
		
		TListenersMap::const_iterator handlersGroupIter = mListenersMap.find(eventType);

		if (handlersGroupIter == mListenersMap.cend()) /// create a new group if it doesn't exists yet
		{
			E_RESULT_CODE result = RC_OK;

			if ((result = _createNewListenersGroup(eventType)) != RC_OK)
			{
				return result;
			}
		}

		mListeners[mListenersMap[eventType]].push_back(pEventListener);

		return RC_OK;
	}

	E_RESULT_CODE CEventManager::Unsubscribe(TypeId eventType, IEventHandler* pEventListener)
	{
		if (!pEventListener)
		{
			return RC_INVALID_ARGS;
		}
		
		TListenersMap::const_iterator handlersGroupIter = mListenersMap.find(eventType);

		if (handlersGroupIter == mListenersMap.cend())
		{
			return RC_FAIL;
		}

		auto& eventListenersGroup = mListeners[(*handlersGroupIter).second];

		auto listenerIter = std::find(eventListenersGroup.cbegin(), eventListenersGroup.cend(), pEventListener);

		if (listenerIter == eventListenersGroup.cend()) /// there is nothing to delete within the group
		{
			return RC_FAIL;
		}

		eventListenersGroup.erase(listenerIter);

		if (eventListenersGroup.empty())
		{
			mFreeGroupsRegistry.push_back((*handlersGroupIter).second);

			mListenersMap.erase(handlersGroupIter);
		}

		return RC_OK;
	}

	E_RESULT_CODE CEventManager::Notify(const TBaseEvent* pEvent)
	{
		if (!pEvent)
		{
			return RC_INVALID_ARGS;
		}

		TListenersMap::const_iterator handlersGroupIter = mListenersMap.find(pEvent->GetEventType());

		if (handlersGroupIter == mListenersMap.cend())
		{
			return RC_FAIL;
		}

		auto currEventListenersGroup = mListeners[(*handlersGroupIter).second];

		IEventHandler* pCurrEventHandler = nullptr;

		for (auto iter = currEventListenersGroup.begin(); iter != currEventListenersGroup.end(); ++iter)
		{
			pCurrEventHandler = *iter;

			if (!pCurrEventHandler || 
				(pEvent->mReceiverId != pCurrEventHandler->GetListenerId() && pEvent->mReceiverId != BroadcastListenersIdValue))
			{
				continue;
			}

			E_RESULT_CODE result = pCurrEventHandler->OnEvent(pEvent);
			TDE2_ASSERT(result == RC_OK);
		}

		return RC_OK;
	}

	E_ENGINE_SUBSYSTEM_TYPE CEventManager::GetType() const
	{
		return EST_EVENT_MANAGER;
	}

	E_RESULT_CODE CEventManager::_createNewListenersGroup(TypeId eventTypeId)
	{
		if (mFreeGroupsRegistry.empty())
		{
			U32 newGroupHash = static_cast<U32>(mListeners.size());

			mListenersMap[eventTypeId] = newGroupHash;

			mListeners.push_back(std::vector<IEventHandler*>());

			return RC_OK;
		}

		U32 newGroupHash = mFreeGroupsRegistry.front();

		mFreeGroupsRegistry.pop_front();

		mListenersMap[eventTypeId] = newGroupHash;
		
		return RC_OK;
	}


	TDE2_API IEventManager* CreateEventManager(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEventManager, CEventManager, result);
	}
}