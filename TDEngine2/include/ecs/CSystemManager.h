/*!
	\file CSystemManager.h
	\date 29.09.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../core/CBaseObject.h"
#include "./../utils/Utils.h"
#include "ISystemManager.h"
#include "./../core/Event.h"
#include <vector>
#include <list>


namespace TDEngine2
{
	class IWorld;
	class ISystem;

	
	/*!
		\brief A factory function for creation objects of CSystemManager's type.

		\param[in, out] pEventManager A pointer to IEventManager implementation
		
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CSystemManager's implementation
	*/

	TDE2_API ISystemManager* CreateSystemManager(IEventManager* pEventManager, E_RESULT_CODE& result);


	/*!
		class CSystemManager

		\brief The implementation of ISystemManager. The manager
		registers, activates and update existing systems.
	*/

	class CSystemManager : public CBaseObject, public ISystemManager, public IEventHandler
	{
		public:
			friend TDE2_API ISystemManager* CreateSystemManager(IEventManager* pEventManager, E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_TYPE(CSystemManager)

			/*!
				\brief The method initializes a ISystemManager's instance

				\param[in, out] pEventManager A pointer to IEventManager implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IEventManager* pEventManager) override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method registers specified system

				\param[in] A pointer to ISystem's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE RegisterSystem(ISystem* pSystem) override;

			/*!
				\brief The method unregisters specified system, but doesn't free its memory

				\param[in] A pointer to ISystem's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE UnregisterSystem(ISystem* pSystem) override;

			/*!
				\brief The method unregisters specified system and free its memory

				\param[in] A pointer to ISystem's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE UnregisterSystemImmediately(ISystem* pSystem) override;

			/*!
				\brief The method marks specified system as an active

				\param[in] A pointer to ISystem's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE ActivateSystem(ISystem* pSystem) override;

			/*!
				\brief The method deactivates specified system

				\param[in] A pointer to ISystem's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE DeactivateSystem(ISystem* pSystem) override;

			/*!
				\brief The main method that should be implemented in all derived classes.
				It contains all the logic that the system will execute during engine's work.

				\param[in] pWorld A pointer to a main scene's object
				\param[in] dt A delta time's value
			*/

			TDE2_API void Update(IWorld* pWorld, F32 dt) override;

			/*!
				\brief The method receives a given event and processes it

				\param[in] pEvent A pointer to event data

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE OnEvent(const TBaseEvent* pEvent) override;

			/*!
				\brief The method returns an identifier of a listener

				\return The method returns an identifier of a listener
			*/

			TDE2_API TEventListenerId GetListenerId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CSystemManager)
		protected:
			std::vector<ISystem*> mpActiveSystems;

			std::list<ISystem*>   mpDeactivatedSystems;

			std::vector<ISystem*> mBuiltinSystems;

			IEventManager*        mpEventManager;
	};
}