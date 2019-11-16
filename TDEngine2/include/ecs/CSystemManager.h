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
#include <unordered_map>
#include <vector>
#include <list>
#include <algorithm>


namespace TDEngine2
{
	class IWorld;
	class ISystem;

	
	/*!
		\brief A factory function for creation objects of CSystemManager's type.

		\param[in, out] pWorld A pointer to IWorld implementation

		\param[in, out] pEventManager A pointer to IEventManager implementation
		
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CSystemManager's implementation
	*/

	TDE2_API ISystemManager* CreateSystemManager(IWorld* pWorld, IEventManager* pEventManager, E_RESULT_CODE& result);


	/*!
		class CSystemManager

		\brief The implementation of ISystemManager. The manager
		registers, activates and update existing systems.
	*/

	class CSystemManager : public CBaseObject, public ISystemManager, public IEventHandler
	{
		public:
			friend TDE2_API ISystemManager* CreateSystemManager(IWorld* pWorld, IEventManager* pEventManager, E_RESULT_CODE& result);
		protected:
			typedef struct TSystemDesc
			{
				U32      mSystemId; /// low bytes contains system's unique id, high bytes contains its priority

				ISystem* mpSystem;
			} TSystemDesc, *TSystemDescPtr;

			typedef std::vector<TSystemDesc>                   TSystemsArray;

			typedef std::list<TSystemDesc>                     TSystemsList;

			typedef std::unordered_map<E_SYSTEM_PRIORITY, U32> TSystemsAccountTable;
		public:
			TDE2_REGISTER_TYPE(CSystemManager)

			/*!
				\brief The method initializes a ISystemManager's instance

				\param[in, out] pWorld A pointer to IWorld implementation

				\param[in, out] pEventManager A pointer to IEventManager implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IWorld* pWorld, IEventManager* pEventManager) override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;
			
			/*!
				\brief The method registers specified system

				\param[in] A pointer to ISystem's implementation

				\param[in] priority A value that represents a priority of a system. Than higher
				priority value then sooner a system will be executed

				\return Either registered system's identifier or an error code
			*/

			TDE2_API TResult<TSystemId> RegisterSystem(ISystem* pSystem, E_SYSTEM_PRIORITY priority = E_SYSTEM_PRIORITY::SP_NORMAL_PRIORITY) override;
			
			/*!
				\brief The method unregisters specified system, but doesn't free its memory

				\param[in] systemId A system's identifier

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE UnregisterSystem(TSystemId systemId) override;

			/*!
				\brief The method unregisters specified system and free its memory

				\param[in] systemId A system's identifier

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE UnregisterSystemImmediately(TSystemId systemId) override;

			/*!
			\brief The method marks specified system as an active

			\param[in] systemId A system's identifier

			\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE ActivateSystem(TSystemId systemId) override;

			/*!
			\brief The method deactivates specified system

			\param[in] systemId A system's identifier

			\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE DeactivateSystem(TSystemId systemId) override;

			/*!
				\brief The method calls ISystem::OnInit method on each system that is currently active

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE InitSystems() override;

			/*!
				\brief The main method that should be implemented in all derived classes.
				It contains all the logic that the system will execute during engine's work.

				\param[in] pWorld A pointer to a main scene's object
				\param[in] dt A delta time's value
			*/

			TDE2_API void Update(IWorld* pWorld, F32 dt) override;

			/*!
				\brief The method calls ISystem::OnDestroy method on each system

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE DestroySystems() override;

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

			template<typename T>
			T _findSystemDesc(T first, T end, TSystemId systemId)
			{
				return std::find_if(first, end, [systemId](const TSystemDesc& systemDesc)
				{
					return systemDesc.mSystemId == systemId;
				});
			}

			TDE2_API E_RESULT_CODE _internalUnregisterSystem(TSystemId systemId);
		protected:
			TSystemsArray        mpActiveSystems;

			TSystemsList         mpDeactivatedSystems;
			
			IEventManager*       mpEventManager;

			IWorld*              mpWorld;

			TSystemsAccountTable mSystemsIdentifiersTable;
	};
}