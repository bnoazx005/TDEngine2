/*!
	\file IWorld.h
	\date 29.09.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../core/IBaseObject.h"
#include "./../utils/Utils.h"
#include "CBaseComponent.h"
#include <functional>
#include <string>


namespace TDEngine2
{
	class ISystem;
	class CEntity;
	class IComponent;
	class IComponentIterator;


	/*!
		interface IWorld

		\brief The interface represents a functionality of scene graph's object
	*/

	class IWorld : public virtual IBaseObject
	{
		public:
			/*!
				\brief The method initializes a world's instance

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init() = 0;

			/*!
				\brief The method creates a new instance of CEntity
				
				\return A pointer to a new instance of CEntity, nullptr may be returned
			*/

			TDE2_API virtual CEntity* CreateEntity() = 0;

			/*!
				\brief The method creates a new instance of CEntity
				
				\param[in] name A name of an entity

				\return A pointer to a new instance of CEntity, nullptr may be returned
			*/

			TDE2_API virtual CEntity* CreateEntity(const std::string& name) = 0;

			/*!
				\brief The method destroys specified entity.
				Note that the entity won't be deleted, it will be
				reused later, so a pointer will be valid.

				\param[in] pEntity A pointer to an entity

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Destroy(CEntity* pEntity) = 0;

			/*!
				\brief The method destroys specified entity
				and frees the memory, that it occupies

				\param[in] pEntity A pointer to an entity

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE DestroyImmediately(CEntity* pEntity) = 0;

			/*!
				\brief The method registers specified system

				\param[in] A pointer to ISystem's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE RegisterSystem(ISystem* pSystem) = 0;

			/*!
				\brief The method unregisters specified system, but doesn't free its memory

				\param[in] A pointer to ISystem's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE UnregisterSystem(ISystem* pSystem) = 0;

			/*!
				\brief The method unregisters specified system and free its memory

				\param[in] A pointer to ISystem's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE UnregisterSystemImmediately(ISystem* pSystem) = 0;

			/*!
				\brief The method marks specified system as an active

				\param[in] A pointer to ISystem's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE ActivateSystem(ISystem* pSystem) = 0;

			/*!
				\brief The method deactivates specified system

				\param[in] A pointer to ISystem's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE DeactivateSystem(ISystem* pSystem) = 0;

			/*!
				\brief The method returns a one way iterator to an array of components of specified type

				\return The method returns a one way iterator to an array of components of specified type
			*/

			template <typename T>
			TDE2_API
#if _HAS_CXX17
				std::enable_if_t<std::is_base_of_v<IComponent, T>, CComponentIterator>
#else
				typename std::enable_if<std::is_base_of<IComponent, T>::value, CComponentIterator>::type
#endif
				FindComponentsOfType()
			{
				return _findComponentsOfType(T::GetTypeId());
			}

			/*!
			\brief The method iterates over each entity, which has specified component

			\param[in] componentTypeId A type of a component

			\param[in] action A callback that will be executed for each entity
			*/

			template <typename T>
			TDE2_API
#if _HAS_CXX17
			std::enable_if_t<std::is_base_of_v<IComponent, T>, void>
#else
			typename std::enable_if<std::is_base_of<IComponent, T>::value, void>::type
#endif
			ForEach(const std::function<void(TEntityId entityId, IComponent* pComponent)>& action)
			{
				return _forEach(T::GetTypeId(), action);
			}

			/*!
				\brief The main method that should be implemented in all derived classes.
				It contains all the logic that the system will execute during engine's work.

				\param[in] dt A delta time's value
			*/

			TDE2_API virtual void Update(float dt) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IWorld)

			TDE2_API virtual CComponentIterator _findComponentsOfType(TypeId typeId) = 0;

			TDE2_API virtual void _forEach(TComponentTypeId componentTypeId, const std::function<void(TEntityId entityId, IComponent* pComponent)>& action) = 0;
	};
}