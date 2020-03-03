/*!
	\file Event.h
	\date 19.11.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "./../utils/Utils.h"


namespace TDEngine2
{
	/*!
		\brief The macro is used to declare virtual method for
		event's type retrieving
	*/

	#define REGISTER_EVENT_TYPE(Type)					\
		TDE2_API virtual TypeId GetEventType() const	\
		{												\
			return Type::GetTypeId();					\
		}

	/*!
		\brief The macro is used to simplify event body's definition
	*/

	#define TDE2_EVENT(EventName)					\
		TDE2_REGISTER_TYPE(EventName)				\
		REGISTER_EVENT_TYPE(EventName)				\
		TDE2_API virtual ~EventName() = default
		


	/*!
		type TBaseEvent

		\brief The s describes a functionality of an event
	*/

	typedef struct TBaseEvent
	{
		TDE2_EVENT(TBaseEvent);

		TEventListenerId mReceiverId = BroadcastListenersIdValue;		///< Receiver's id, use BroadcastListenersIdValue for event's broadcasting
	} TBaseEvent, *TBaseEventPtr;


	/*!
		interface IEventHandler

		\brief The interface represents an empty event handler's functionality
	*/

	class IEventHandler
	{
		public:
			/*!
				\brief The method receives a given event and processes it

				\param[in] pEvent A pointer to event data

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE OnEvent(const TBaseEvent* pEvent) = 0;
			
			/*!
				\brief The method returns an identifier of a listener

				\return The method returns an identifier of a listener
			*/

			TDE2_API virtual TEventListenerId GetListenerId() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IEventHandler)
	};
}