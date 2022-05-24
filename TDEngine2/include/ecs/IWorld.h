/*!
	\file IWorld.h
	\date 29.09.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "../core/IBaseObject.h"
#include "../utils/Utils.h"
#include "../utils/Types.h"
#include "CEntity.h"
#include "CBaseComponent.h"
#include <functional>
#include <string>
#include <vector>
#include <unordered_map>
#include <stack>


namespace TDEngine2
{
	class ISystem;
	class CEntity;
	class IComponent;
	class IComponentIterator;
	class IEventManager;
	class IRaycastContext;
	class CTransform;


	TDE2_DECLARE_SCOPED_PTR(IEventManager)
	TDE2_DECLARE_SCOPED_PTR(IRaycastContext)


	/*!
		struct TComponentsQueryLocalSlice

		\brief The type is used to retrieve local independent copy of a part of components arrays. It can be used
		to speed up iteration over them than same GetComponent<> invokations.
	*/


	template <typename... TArgs>
	struct TComponentsQueryLocalSlice
	{
		TDE2_API TDE2_STATIC_CONSTEXPR USIZE mInvalidParentIndex = (std::numeric_limits<USIZE>::max)();

		USIZE                                mComponentsCount = 0;

		std::vector<USIZE>                   mParentsToChildMapping; ///< Contains indices of parents for each element of a components array
		std::tuple<std::vector<TArgs*>...>   mComponentsSlice;
	};


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

			TDE2_API virtual E_RESULT_CODE Init(TPtr<IEventManager> pEventManager) = 0;

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
				\brief The method implements the logic that should be done before the object will be released
				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE OnBeforeFree() = 0;

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
				return _findSystem(T::GetTypeId());
			}
			
			/*!
				\brief The method allows to iterate over all registered systems including deactivated ones
			*/

			TDE2_API virtual void ForEachSystem(const std::function<void(TSystemId, const ISystem* const)> action = nullptr) const = 0;


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
			TDE2_API std::vector<TEntityId> FindEntitiesWithAny()
			{
				return _findEntitiesWithAnyComponents({ { TArgs::GetTypeId()... } });
			}

			/*!
				\brief The method seeks out an entity and either return it or return nullptr

				\param[in] entityId Unique entity's identifier

				\return The method seeks out an entity and either return it or return nullptr
			*/

			TDE2_API virtual CEntity* FindEntity(TEntityId entityId) const = 0;

			template <typename TComponentType>
			TDE2_API TEntityId FindEntityWithUniqueComponent()
			{
				return _findEntityWithUniqueComponent(TComponentType::GetTypeId());
			}

			template <typename... TArgs>
			TDE2_API TComponentsQueryLocalSlice<TArgs...> CreateLocalComponentsSlice()
			{
				std::vector<TEntityId> entities = FindEntitiesWithComponents<TArgs...>();
				
				TComponentsQueryLocalSlice<TArgs...> result;

				if (TContainsType<CTransform, TArgs...>::mValue) /// \note For CTransform we should sort all entities that parents should preceede their children
				{
					/// \note Fill up relationships table to sort entities based on their dependencies 
					std::unordered_map<TEntityId, std::vector<TEntityId>> parentToChildRelations;

					for (TEntityId currEntityId : entities)
					{
						if (CEntity* pEntity = FindEntity(currEntityId))
						{
							parentToChildRelations[pEntity->GetComponent<CTransform>()->GetParent()].push_back(pEntity->GetId());
						}
					}

					entities.clear();

					std::stack<std::tuple<TEntityId, USIZE>> entitiesToProcess;

					for (TEntityId currEntityId : parentToChildRelations[TEntityId::Invalid])
					{
						entitiesToProcess.push({ currEntityId, TComponentsQueryLocalSlice<TArgs...>::mInvalidParentIndex });
					}

					TEntityId currEntityId;
					USIZE currParentElementIndex = 0;

					while (!entitiesToProcess.empty())
					{
						std::tie(currEntityId, currParentElementIndex) = entitiesToProcess.top();
						entitiesToProcess.pop();

						result.mParentsToChildMapping.push_back(currParentElementIndex);
						entities.push_back(currEntityId);

						const USIZE parentIndex = entities.size() - 1;

						for (TEntityId currEntityId : parentToChildRelations[currEntityId])
						{
							entitiesToProcess.push({ currEntityId, parentIndex });
						}
					}
				}
				
				result.mComponentsSlice = std::make_tuple(_getComponentsOfTypeFromEntities<TArgs>(entities)...);
				result.mComponentsCount = entities.size();

				return std::move(result);
			}
			
			/*!
				\brief The method registers given raycasting context within the world's instance

				\param[in, out] pRaycastContext A pointer to IRaycastContext implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE RegisterRaycastContext(TPtr<IRaycastContext> pRaycastContext) = 0;

			/*!
				\brief The method sets up time scale factor which impacts on update cycles of all entities and systems

				\param[in] scaleFactor Could be positive, negative and zero value. The latter means some kind of pause
				for the world
			*/

			TDE2_API virtual void SetTimeScaleFactor(F32 scaleFactor) = 0;

			/*!
				\brief The main method that should be implemented in all derived classes.
				It contains all the logic that the system will execute during engine's work.

				\param[in] dt A delta time's value
			*/

			TDE2_API virtual void Update(F32 dt) = 0;

			/*!
				\brief The method returns a pointer to IRaycastContext, use

				\return The method returns a pointer to IRaycastContext
			*/

			TDE2_API virtual TPtr<IRaycastContext> GetRaycastContext() const = 0;

			TDE2_API virtual CEntityManager* GetEntityManager() const = 0;

			TDE2_API virtual F32 GetTimeScaleFactor() const = 0;

#if TDE2_EDITORS_ENABLED
			TDE2_API virtual const std::vector<std::string>& GetRegisteredComponentsIdentifiers() const = 0;
#endif
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IWorld)

			TDE2_API virtual CComponentIterator _findComponentsOfType(TypeId typeId) = 0;

			TDE2_API virtual void _forEach(TypeId componentTypeId, const std::function<void(TEntityId entityId, IComponent* pComponent)>& action) = 0;

			TDE2_API virtual std::vector<TEntityId> _findEntitiesWithComponents(const std::vector<TypeId>& types) = 0;

			TDE2_API virtual std::vector<TEntityId> _findEntitiesWithAnyComponents(const std::vector<TypeId>& types) = 0;

			TDE2_API virtual TEntityId _findEntityWithUniqueComponent(TypeId typeId) = 0;

			TDE2_API virtual TSystemId _findSystem(TypeId typeId) = 0;

			template <typename TComponentType>
			TDE2_API std::vector<TComponentType*> _getComponentsOfTypeFromEntities(const std::vector<TEntityId>& entities)
			{
				std::vector<TComponentType*> components;

				for (TEntityId currEntityId : entities)
				{
					if (auto&& pEntity = FindEntity(currEntityId))
					{
						if (TComponentType* pComponent = pEntity->GetComponent<TComponentType>())
						{
							components.push_back(pComponent);
						}
					}
				}

				return std::move(components);
			}
	};


	TDE2_DECLARE_SCOPED_PTR_INLINED(IWorld)
}