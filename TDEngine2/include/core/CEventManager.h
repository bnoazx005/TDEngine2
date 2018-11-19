/*!
	\file CEventManager.h
	\date 19.11.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "IEventManager.h"
#include <unordered_map>
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

	class CEventManager : public IEventManager
	{
		public:
			friend TDE2_API IEventManager* CreateEventManager(E_RESULT_CODE& result);
		protected:
			typedef std::unordered_map<TypeId, U32>           TListenersMap;

			typedef std::vector<std::vector<IEventHandler*>>  TListenersMatrix;

			typedef std::list<U32>                            TFreeGroupsRegistry;
		public:
			/*!
				\brief The method initializes a file system's object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init() override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method registers a new subscriber into a list of subscribers

				\param[in, out] pEventListener A pointer to implementation of a specific event listener

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Subscribe(IEventHandler* pEventListener) override;

			/*!
				\brief The method unregisters a given subscriber from a list of subscribers

				\param[in, out] pEventListener A pointer to implementation of a specific event listener

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Unsubscribe(IEventHandler* pEventListener) override;

			/*!
				\brief The method broadcasts a given event to its listeners

				\param[in] pEvent A pointer to event data

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Notify(const TBaseEvent* pEvent) override;

			/*!
				\brief The method returns a type of the subsystem

				\return A type, which is represented with E_ENGINE_SUBSYSTEM_TYPE's value
			*/

			TDE2_API E_ENGINE_SUBSYSTEM_TYPE GetType() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CEventManager)

			TDE2_API E_RESULT_CODE _createNewListenersGroup(TypeId eventTypeId);
		protected:
			bool                mIsInitialized;

			TListenersMap       mListenersMap;

			TListenersMatrix    mListeners;

			TFreeGroupsRegistry mFreeGroupsRegistry;
	};
}
