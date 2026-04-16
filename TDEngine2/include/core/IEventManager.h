/*!
	\file IEventManager.h
	\date 19.11.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "IEngineSubsystem.h"
#include "../utils/Utils.h"


namespace TDEngine2
{
	struct TBaseEvent;

	class IEventHandler;
	class IEventManager;


	class IEventQueue
	{
		public:
			virtual ~IEventQueue() = default;

			virtual void Flush(IEventManager& manager) = 0;
			virtual void Reserve(USIZE capacity = 64) = 0;
	};


	template <typename TEventType> class CEventQueue;


	/*!
		interface IEventManager

		\brief The interface describes a functionality that any event manager
		in the engine should provide.
	*/

	class IEventManager : public IEngineSubsystem
	{
		public:
			/*!
				\brief The method initializes a file system's object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init() = 0;

			/*!
				\brief The method registers a new subscriber into a list of subscribers

				\param[in] eventType A type of an event that a subscriber want to listen to

				\param[in, out] pEventListener A pointer to implementation of a specific event listener

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Subscribe(TypeId eventType, IEventHandler* pEventListener) = 0;

			/*!
				\brief The method unregisters a given subscriber from a list of subscribers

				\param[in] eventType A type of an event

				\param[in, out] pEventListener A pointer to implementation of a specific event listener

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Unsubscribe(TypeId eventType, IEventHandler* pEventListener) = 0;

			template <typename TEventType>
			std::enable_if_t<std::is_base_of_v<TBaseEvent, TEventType>, E_RESULT_CODE> EnableBufferingForEventType()
			{
				return _enableBufferingForEventTypeImpl(TEventType::GetTypeId());
			}

			/*!
				\brief The method broadcasts a given event to its listeners. The moment of notification differs based on fact 
				whether or not given event type supports buffering

				\param[in] event A reference to triggered event that contains all its data

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			template <typename TEventType>
			std::enable_if_t<std::is_base_of_v<TBaseEvent, TEventType>, E_RESULT_CODE> Notify(const TEventType& event)
			{
				const TypeId eventTypeId = TEventType::GetTypeId();

				if (_isEventTypeSupportBuffering(eventTypeId))
				{
					IEventQueue* pEventQueue = _getEventQueueByType(eventTypeId);
					if (!pEventQueue)
					{
						_registerEventQueue(eventTypeId, std::make_unique<CEventQueue<TEventType>>());
						pEventQueue = _getEventQueueByType(eventTypeId);
					}

					CEventQueue<TEventType>* pConcreteEventQueue = static_cast<CEventQueue<TEventType>*>(pEventQueue);
					if (!pConcreteEventQueue)
					{
						return RC_FAIL;
					}

					pConcreteEventQueue->Push(event);

					return RC_OK;
				}

				return NotifyImmediate(event);
			}

			/*!
				\brief The method broadcasts a given event to its listeners just in time no matter of whether a given event type
				supports buffering or not

				\param[in] event A reference to triggered event that contains all its data

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE NotifyImmediate(const TBaseEvent& event) = 0;

			TDE2_API static E_ENGINE_SUBSYSTEM_TYPE GetTypeID() { return EST_EVENT_MANAGER; }
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IEventManager)

			TDE2_API virtual E_RESULT_CODE _registerEventQueue(TypeId eventTypeId, std::unique_ptr<IEventQueue> pEventQueue) = 0;
			TDE2_API virtual IEventQueue* _getEventQueueByType(TypeId eventTypeId) = 0;

			TDE2_API virtual E_RESULT_CODE _enableBufferingForEventTypeImpl(TypeId eventTypeId) = 0;
			TDE2_API virtual bool _isEventTypeSupportBuffering(TypeId eventTypeId) const = 0;
	};


	TDE2_DECLARE_SCOPED_PTR(IEventManager)


	template <typename TEventType>
	class CEventQueue : public IEventQueue
	{
		public:
			void Flush(IEventManager& manager) override
			{
				for (const TEventType& event : mEventsBuffer)
				{
					manager.NotifyImmediate(event);
				}

				mEventsBuffer.clear();
			}

			void Reserve(USIZE capacity = 64) override
			{
				mEventsBuffer.reserve(capacity);
			}

			void Push(const TEventType& event)
			{
				mEventsBuffer.emplace_back(event);
			}
		private:
			Vector<TEventType> mEventsBuffer{};
	};
}
