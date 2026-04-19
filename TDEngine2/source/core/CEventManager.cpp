#include "../../include/core/CEventManager.h"
#include "../../include/core/Event.h"
#include "../../include/editor/CPerfProfiler.h"
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

	E_RESULT_CODE CEventManager::NotifyImmediate(const TBaseEvent& event)
	{
		TDE2_PROFILER_SCOPE("CEventManager::NotifyImmediate");

		TListenersMap::const_iterator handlersGroupIter = mListenersMap.find(event.GetEventType());
		if (handlersGroupIter == mListenersMap.cend())
		{
			return RC_FAIL;
		}

		for (IEventHandler* pCurrEventHandler : mListeners[(*handlersGroupIter).second])
		{
			if (!pCurrEventHandler || (event.mReceiverId != pCurrEventHandler->GetListenerId() && event.mReceiverId != BroadcastListenersIdValue))
			{
				continue;
			}

			[[maybe_unused]] E_RESULT_CODE result = pCurrEventHandler->OnEvent(event);
			TDE2_ASSERT(result == RC_OK);
		}

		return RC_OK;
	}

	void CEventManager::FlushAll()
	{
		TDE2_PROFILER_SCOPE("CEventManager::FlushAll");

		for (const auto& eventQueueEntry : mEventQueues)
		{
			eventQueueEntry.second->Flush(*this);
		}
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

			mListeners.emplace_back();

			return RC_OK;
		}

		U32 newGroupHash = mFreeGroupsRegistry.front();

		mFreeGroupsRegistry.pop_front();

		mListenersMap[eventTypeId] = newGroupHash;
		
		return RC_OK;
	}

	E_RESULT_CODE CEventManager::_registerEventQueue(TypeId eventTypeId, std::unique_ptr<IEventQueue> pEventQueue)
	{
		TDE2_PROFILER_SCOPE("CEventManager::_registerEventQueue");

		if (!pEventQueue)
		{
			return RC_INVALID_ARGS;
		}

		pEventQueue->Reserve();
		mEventQueues.emplace(eventTypeId, std::move(pEventQueue));

		return RC_OK;
	}

	IEventQueue* CEventManager::_getEventQueueByType(TypeId eventTypeId)
	{
		auto&& it = mEventQueues.find(eventTypeId);
		return (it == mEventQueues.cend()) ? nullptr : it->second.get();
	}

	E_RESULT_CODE CEventManager::_enableBufferingForEventTypeImpl(TypeId eventTypeId)
	{
		if (TypeId::Invalid == eventTypeId)
		{
			return RC_INVALID_ARGS;
		}

		mEventTypesWithBufferingTable.emplace(eventTypeId);
		return RC_OK;
	}

	bool CEventManager::_isEventTypeSupportBuffering(TypeId eventTypeId) const
	{
		return mEventTypesWithBufferingTable.find(eventTypeId) != mEventTypesWithBufferingTable.cend();
	}

	E_RESULT_CODE CEventManager::_flushImpl(TypeId eventTypeId)
	{
		TDE2_PROFILER_SCOPE("CEventManager::_flushImpl");

		IEventQueue* pEventQueue = _getEventQueueByType(eventTypeId);
		if (!pEventQueue)
		{
			return RC_FAIL;
		}

		pEventQueue->Flush(*this);

		return RC_OK;
	}


	TDE2_API IEventManager* CreateEventManager(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEventManager, CEventManager, result);
	}
}