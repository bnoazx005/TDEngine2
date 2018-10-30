/*!
	\file CWorld.h
	\date 29.09.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../core/CBaseObject.h"
#include "IWorld.h"


namespace TDEngine2
{
	class ISystem;
	class CEntity;
	class CEntityManager;
	class IComponentManager;
	class ISystemManager;


	/*!
		interface CWorld

		\brief The class is a main scene object, which
		manages all existing entities and components
	*/

	class CWorld : public CBaseObject, public IWorld
	{
		public:
			friend TDE2_API IWorld* CreateWorld(E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes a world's instance

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init() override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method creates a new instance of CEntity

				\return A pointer to a new instance of CEntity, nullptr may be returned
			*/

			TDE2_API CEntity* CreateEntity() override;

			/*!
				\brief The method destroys specified entity.
				Note that the entity won't be deleted, it will be
				reused later, so a pointer will be valid.

				\param[in] pEntity A pointer to an entity

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Destroy(CEntity* pEntity) override;

			/*!
				\brief The method destroys specified entity
				and frees the memory, that it occupies

				\param[in] pEntity A pointer to an entity

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE DestroyImmediately(CEntity* pEntity) override;

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

				\param[in] dt A delta time's value
			*/

			TDE2_API void Update(float dt) override;
		protected:
			TDE2_API CWorld();
			TDE2_API ~CWorld() = default;
			TDE2_API CWorld(const CWorld& world) = delete;
			TDE2_API CWorld& operator=(const CWorld& world) = delete;
		protected:
			CEntityManager*    mpEntityManager;
			IComponentManager* mpComponentManager;
			ISystemManager*    mpSystemManager;
	};


	/*!
		\brief A factory function for creation objects of CWorld's type.

		\return A pointer to CWorld's implementation
	*/

	TDE2_API IWorld* CreateWorld(E_RESULT_CODE& result);
}