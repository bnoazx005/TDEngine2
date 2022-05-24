/*!
	\file IComponentManager.h
	\date 30.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../core/IBaseObject.h"
#include "./../utils/Utils.h"
#include "CBaseComponent.h"
#include <functional>
#include <vector>


namespace TDEngine2
{
	class IComponentFactory;


	/*!
		interface IComponentManager

		\brief The interface describes a functionality of a component manager, which
		creates, destroys and stores all components in the engine
	*/

	class IComponentManager : public virtual IBaseObject
	{
		public:
			/*!
				\brief The method initializes a component manager's instance

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init() = 0;

			/*!
				\brief The method registers specified resource factory within a manager

				\param[in] pFactory A pointer to IComponentFactory's implementation

				\return The method returns an object, which contains a status of method's execution and
				an identifier of the registred factory
			*/

			TDE2_API virtual E_RESULT_CODE RegisterFactory(TPtr<IComponentFactory> pFactory) = 0;

			/*!
				\brief The method unregisters a component factory with the specified identifier

				\param[in] resourceFactoryId An identifier of a component factory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			template <typename T>
			TDE2_API
	#if _HAS_CXX17
				std::enable_if_t<std::is_base_of_v<IComponent, T>, E_RESULT_CODE>
	#else
				typename std::enable_if<std::is_base_of<IComponent, T>::value, E_RESULT_CODE>::type
	#endif
			UnregisterFactory()
			{
				return _unregisterFactory(T::GetTypeId());
			}


			/*!
				\brief The method creates a new component and connects it with
				the entity

				\return A pointer to a component, or nullptr if some error has occured
			*/

			template <typename T>
			TDE2_API
	#if _HAS_CXX17
				std::enable_if_t<std::is_base_of_v<IComponent, T>, T*>
	#else
				typename std::enable_if<std::is_base_of<IComponent, T>::value, T*>::type
	#endif
				CreateComponent(TEntityId id)
			{
				return dynamic_cast<T*>(_createComponent(T::GetTypeId(), id));
			}

			/*!
				\brief The method creates a new component based on a given identifier of the entity and component's type

				\param[in] entityId An identifier of an entity
				\param[in] componentTypeId A type's identifier of the component

				\return A pointer to a component, or nullptr if some error has occured
			*/

			TDE2_API virtual IComponent* CreateComponent(TEntityId entityId, TypeId componentTypeId) = 0;

			/*!
				\brief The method removes a component of specified T type.
				This method doesn't free the memory that is occupied by a component.

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			template <typename T>
			TDE2_API
	#if _HAS_CXX17
			std::enable_if_t<std::is_base_of_v<IComponent, T>, E_RESULT_CODE>
	#else
			typename std::enable_if<std::is_base_of<IComponent, T>::value, E_RESULT_CODE>::type
	#endif
			RemoveComponent(TEntityId id)
			{
				return RemoveComponent(T::GetTypeId(), id);
			}

			TDE2_API virtual E_RESULT_CODE RemoveComponent(TypeId componentTypeId, TEntityId entityId) = 0;

			/*!
				\brief The method removes a component of specified T type and
				frees the memory occupied by it

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			template <typename T>
			TDE2_API
	#if _HAS_CXX17
			std::enable_if_t<std::is_base_of_v<IComponent, T>, E_RESULT_CODE>
	#else
			typename std::enable_if<std::is_base_of<IComponent, T>::value, E_RESULT_CODE>::type
	#endif
			RemoveComponentImmediately(TEntityId id)
			{
				return _removeComponentImmediately(T::GetTypeId(), id);
			}

			/*!
				\brief The method removes all components that are related with the entity.
				This method doesn't free the memory that is occupied by components.

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE RemoveComponents(TEntityId id) = 0;

			/*!
				\return The method returns an array of components that belong to given entity
			*/

			TDE2_API virtual std::vector<IComponent*> GetComponents(TEntityId id) const = 0;

			/*!
				\brief The method removes all components that are related with the entity and
				frees the memory occupied by them.

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE RemoveComponentsImmediately(TEntityId id) = 0;
			
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
				return FindComponentsOfType(T::GetTypeId());
			}
			
			/*!
				\brief The method returns a one way iterator to an array of components of specified type

				\param[in] typeId A type of a component

				\return The method returns a one way iterator to an array of components of specified type
			*/

			TDE2_API virtual CComponentIterator FindComponentsOfType(TypeId typeId) = 0;

			/*!
				\brief The method iterates over each entity, which has specified component

				\param[in] componentTypeId A type of a component

				\param[in] action A callback that will be executed for each entity
			*/

			TDE2_API virtual void ForEach(TypeId componentTypeId, const std::function<void(TEntityId entityId, IComponent* pComponent)>& action) = 0;

			/*!
				\brief The method returns an array of entities identifiers, which have all of
				specified components

				\param[in] types An array that contains types identifiers that an entity should have

				\return The method returns an array of entities identifiers, which have all of
				specified components
			*/

			TDE2_API virtual std::vector<TEntityId> FindEntitiesWithAll(const std::vector<TypeId>& types) = 0;

			/*!
				\brief The method returns an array of entities identifiers, which have any of
				specified components

				\param[in] types An array that contains types identifiers that an entity should have

				\return The method returns an array of entities identifiers, which have any of
				specified components
			*/

			TDE2_API virtual std::vector<TEntityId> FindEntitiesWithAny(const std::vector<TypeId>& types) = 0;

			/*!
				\param[in] types An array that contains types identifiers that an entity should have. Note that the method
				isn't responsible for creating a new instances of unqiue components.
				
				\return The method returns an entity which holds a unique component
			*/

			TDE2_API virtual TEntityId FindEntityWithUniqueComponent(TypeId typeId) = 0;

			/*!
				\brief The method returns a pointer to a component of specified type T

				\return The method returns a pointer to a component of specified type T, or nullptr if there is no
				attached component of desired type.
			*/

			template <typename T>
			TDE2_API
	#if _HAS_CXX17
				std::enable_if_t<std::is_base_of_v<IComponent, T>, T*>
	#else
				typename std::enable_if<std::is_base_of<IComponent, T>::value, T*>::type
	#endif
				GetComponent(TEntityId id)
			{
				return dynamic_cast<T*>(_getComponent(T::GetTypeId(), id));
			}

			/*!
				\brief The method checks up whether a given entity has a component of specified type or not

				\return The method returns true if the entity has specified component and false in other cases
			*/

			template <typename T>
			TDE2_API bool HasComponent(TEntityId id)
			{
				return _hasComponent(T::GetTypeId(), id);
			}

#if TDE2_EDITORS_ENABLED
			TDE2_API virtual const std::vector<std::string>& GetRegisteredComponentsIdentifiers() const = 0;
#endif
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IComponentManager)

			TDE2_API virtual E_RESULT_CODE _unregisterFactory(TypeId typeId) = 0;

			TDE2_API virtual IComponent* _createComponent(TypeId componentTypeId, TEntityId entityId) = 0;

			TDE2_API virtual IComponent* _getComponent(TypeId componentTypeId, TEntityId entityId) = 0;

			TDE2_API virtual E_RESULT_CODE _removeComponentImmediately(TypeId componentTypeId, TEntityId entityId) = 0;

			TDE2_API virtual bool _hasComponent(TypeId componentTypeId, TEntityId entityId) = 0;
	};


	TDE2_DECLARE_SCOPED_PTR_INLINED(IComponentManager)
}
