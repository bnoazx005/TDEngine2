/*!
	\file IWorld.h
	\date 29.09.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../core/IBaseObject.h"
#include "./../utils/Utils.h"
#include "./../utils/CResult.h"
#include "./../utils/Types.h"
#include "CBaseComponent.h"
#include <functional>
#include <string>
#include <vector>


namespace TDEngine2
{
	class ISystem;
	class CEntity;
	class IComponent;
	class IComponentIterator;
	class IEventManager;


	/*!
		interface IWorld

		\brief The interface represents a functionality of scene graph's object
	*/

	class IWorld : public virtual IBaseObject
	{
		public:
			/*!
				\brief The method initializes a world's instance

				\param[in, out] A pointer to IEventManager implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IEventManager* pEventManager) = 0;

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
				\brief The method returns an array of entities identifiers, which have all of
				specified components

				\return The method returns an array of entities identifiers, which have all of
				specified components
			*/

			template <typename... TArgs>
			TDE2_API std::vector<TEntityId> FindEntitiesWithComponents()
			{
				/// \todo this implementation is not safe enough, another solution should be found instead
				return _findEntitiesWithComponents({ { TArgs::GetTypeId()... } });
			}

			/*!
				\brief The method returns an array of entities identifiers, which have any component
				from given arguments

				\return The method returns an array of entities identifiers, which have any component
				from given arguments
			*/

			template <typename... TArgs>
			TDE2_API TEntityId FindEntitiesWithAny()
			{
				return _findEntityWithAnyComponents({ { TArgs::GetTypeId()... } });
			}

			/*!
				\brief The method seeks out an entity and either return it or return nullptr

				\param[in] entityId Unique entity's identifier

				\return The method seeks out an entity and either return it or return nullptr
			*/

			TDE2_API virtual CEntity* FindEntity(TEntityId entityId) const = 0;
			
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

			TDE2_API virtual std::vector<TEntityId> _findEntitiesWithComponents(const std::vector<TComponentTypeId>& types) = 0;

			TDE2_API virtual std::vector<TEntityId> _findEntitiesWithAnyComponents(const std::vector<TComponentTypeId>& types) = 0;
	};
}