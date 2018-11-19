/*!
	\file IEventManager.h
	\date 19.11.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "IEngineSubsystem.h"
#include "./../utils/Utils.h"


namespace TDEngine2
{
	struct TBaseEvent;

	class IEventHandler;


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

				\param[in, out] pEventListener A pointer to implementation of a specific event listener

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Subscribe(IEventHandler* pEventListener) = 0;

			/*!
				\brief The method unregisters a given subscriber from a list of subscribers

				\param[in, out] pEventListener A pointer to implementation of a specific event listener

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Unsubscribe(IEventHandler* pEventListener) = 0;

			/*!
				\brief The method broadcasts a given event to its listeners

				\param[in] pEvent A pointer to event data

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Notify(const TBaseEvent* pEvent) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IEventManager)
	};
}
