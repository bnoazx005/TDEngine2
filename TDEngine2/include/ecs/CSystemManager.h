/*!
	\file CSystemManager.h
	\date 29.09.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../core/CBaseObject.h"
#include "./../utils/Utils.h"
#include "ISystemManager.h"
#include <vector>
#include <list>


namespace TDEngine2
{
	class IWorld;
	class ISystem;


	/*!
		class CSystemManager

		\brief The implementation of ISystemManager. The manager
		registers, activates and update existing systems.
	*/

	class CSystemManager : public CBaseObject, public ISystemManager
	{
		public:
			friend TDE2_API ISystemManager* CreateSystemManager(E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes a ISystemManager's instance

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init() override;

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
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CSystemManager)
		protected:
			std::vector<ISystem*> mpActiveSystems;

			std::list<ISystem*>   mpDeactivatedSystems;

			std::vector<ISystem*> mBuiltinSystems;
	};


	/*!
		\brief A factory function for creation objects of CSystemManager's type.

		\return A pointer to CSystemManager's implementation
	*/

	TDE2_API ISystemManager* CreateSystemManager(E_RESULT_CODE& result);
}