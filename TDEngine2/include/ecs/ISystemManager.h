/*!
	\file ISystemManager.h
	\date 29.09.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "../core/IBaseObject.h"
#include "../utils/Utils.h"


namespace TDEngine2
{
	class ISystem;
	class IWorld;
	class IEventManager;
	class IJobManager;


	TDE2_DECLARE_SCOPED_PTR(IJobManager)
	TDE2_DECLARE_SCOPED_PTR(ISystem)

	/*!
		interface ISystemManager

		\brief The interface describes a functionality of a system manager.
		Main tasks are registration, activation and processing existing systems.
	*/
	
	class ISystemManager: public virtual IBaseObject
	{
		public:
			/*!
				\brief The method initializes a ISystemManager's instance

				\param[in, out] pWorld A pointer to IWorld implementation
				\param[in, out] pEventManager A pointer to IEventManager implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IWorld* pWorld, IEventManager* pEventManager, TPtr<IJobManager> pJobManager) = 0;

			/*!
				\brief The method registers specified system
				
				\param[in] A pointer to ISystem's implementation

				\param[in] priority A value that represents a priority of a system. Than higher
				priority value then sooner a system will be executed

				\return Either registered system's identifier or an error code
			*/
			
			TDE2_API virtual TResult<TSystemId> RegisterSystem(ISystem* pSystem, E_SYSTEM_PRIORITY priority = E_SYSTEM_PRIORITY::SP_NORMAL_PRIORITY) = 0;

			/*!
				\brief The method unregisters specified system, but doesn't free its memory

				\param[in] systemId A system's identifier

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE UnregisterSystem(TSystemId systemId) = 0;

			/*!
				\brief The method unregisters specified system and free its memory

				\param[in] systemId A system's identifier

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE UnregisterSystemImmediately(TSystemId systemId) = 0;

			/*!
				\brief The method marks specified system as an active

				\param[in] systemId A system's identifier

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE ActivateSystem(TSystemId systemId) = 0;

			/*!
				\brief The method deactivates specified system

				\param[in] systemId A system's identifier

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE DeactivateSystem(TSystemId systemId) = 0;

			/*!
				\brief The method calls ISystem::OnInit method on each system that is currently active
			
				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE InitSystems() = 0;

			/*!
				\brief The main method that should be implemented in all derived classes.
				It contains all the logic that the system will execute during engine's work.

				\param[in] pWorld A pointer to a main scene's object
				\param[in] dt A delta time's value
			*/

			TDE2_API virtual void Update(IWorld* pWorld, F32 dt) = 0;

			/*!
				\brief The method unregisters and frees all existing systems
				
				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE ClearSystemsRegistry() = 0;

			/*!
				\brief The method calls ISystem::OnDestroy method on each system

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE DestroySystems() = 0;

			/*!
				\brief The method returns an identifier of a registered system by its type. IF there is no
				active system of given type then InvalidSystemId is returned

				\return The method returns an identifier of a registered system by its type
			*/

			template <typename T>
			TDE2_API
#if _HAS_CXX17
				std::enable_if_t<std::is_base_of_v<ISystem, T>, TSystemId>
#else
				typename std::enable_if<std::is_base_of<ISystem, T>::value, TSystemId>::type
#endif
				FindSystem()
			{
				return FindSystem(T::GetTypeId());
			}

			/*!
				\brief The method returns an identifier of a registered system by its type. IF there is no
				active system of given type then InvalidSystemId is returned

				\param[in] systemTypeId A system type's identifier

				\return The method returns an identifier of a registered system by its type
			*/

			TDE2_API virtual TSystemId FindSystem(TypeId systemTypeId) = 0;

			TDE2_API virtual TPtr<ISystem> GetSystem(TSystemId handle) = 0;

			/*!
				\brief The method allows to iterate over all registered systems including deactivated ones
			*/

			TDE2_API virtual void ForEachSystem(const std::function<void(TSystemId, const ISystem* const)> action = nullptr) const = 0;

			TDE2_API virtual bool IsSystemActive(TSystemId systemId) const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ISystemManager)
	};
}