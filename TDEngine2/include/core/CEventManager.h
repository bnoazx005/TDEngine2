/*!
	\file CEventManager.h
	\date 19.11.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "IEventManager.h"
#include "CBaseObject.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <list>


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CEventManager's type.
		
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CEventManager's implementation
	*/

	TDE2_API IEventManager* CreateEventManager(E_RESULT_CODE& result);


	/*!
		class CEventManager

		\brief The interface describes a functionality that any event manager
		in the engine should provide.
	*/

	class CEventManager : public IEventManager, public CBaseObject
	{
		public:
			friend TDE2_API IEventManager* CreateEventManager(E_RESULT_CODE& result);
		protected:
			typedef std::unordered_map<TypeId, U32>                          TListenersMap;

			typedef Vector<IEventHandler*>                                   TEventHandlersArray;
			typedef Vector<TEventHandlersArray>                              TListenersMatrix;

			typedef std::list<U32>                                           TFreeGroupsRegistry;

			typedef std::unordered_set<TypeId>                               TTypesTable;

			typedef std::unordered_map<TypeId, std::unique_ptr<IEventQueue>> TEventQueuesTable;
		public:
			/*!
				\brief The method initializes a file system's object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init() override;

			/*!
				\brief The method registers a new subscriber into a list of subscribers

				\param[in] eventType A type of event that a subscriber want to listen to

				\param[in, out] pEventListener A pointer to implementation of a specific event listener

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Subscribe(TypeId eventType, IEventHandler* pEventListener) override;

			/*!
				\brief The method unregisters a given subscriber from a list of subscribers

				\param[in] eventType A type of an event

				\param[in, out] pEventListener A pointer to implementation of a specific event listener

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Unsubscribe(TypeId eventType, IEventHandler* pEventListener) override;

			/*!
				\brief The method broadcasts a given event to its listeners just in time no matter of whether a given event type
				supports buffering or not

				\param[in] event A reference to triggered event that contains all its data

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE NotifyImmediate(const TBaseEvent& event) override;

			TDE2_API void FlushAll() override;

			/*!
				\brief The method returns a type of the subsystem

				\return A type, which is represented with E_ENGINE_SUBSYSTEM_TYPE's value
			*/

			TDE2_API E_ENGINE_SUBSYSTEM_TYPE GetType() const override;

			TDE2_API static E_ENGINE_SUBSYSTEM_TYPE GetTypeID() { return EST_EVENT_MANAGER; }
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CEventManager)

			E_RESULT_CODE _createNewListenersGroup(TypeId eventTypeId);

			E_RESULT_CODE _registerEventQueue(TypeId eventTypeId, std::unique_ptr<IEventQueue> pEventQueue) override;
			IEventQueue* _getEventQueueByType(TypeId eventTypeId) override;

			E_RESULT_CODE _enableBufferingForEventTypeImpl(TypeId eventTypeId) override;
			bool _isEventTypeSupportBuffering(TypeId eventTypeId) const override;

			E_RESULT_CODE _flushImpl(TypeId eventTypeId) override;
		protected:
			TListenersMap       mListenersMap;

			TListenersMatrix    mListeners;

			TFreeGroupsRegistry mFreeGroupsRegistry;

			TTypesTable         mEventTypesWithBufferingTable{};

			TEventQueuesTable   mEventQueues{};
	};
}
