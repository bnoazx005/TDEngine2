/*!
	\file CComponentManager.h
	\date 28.09.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "../core/CBaseObject.h"
#include "../ecs/IComponentManager.h"
#include "../utils/Utils.h"
#include <vector>
#include <list>
#include <unordered_set>
#include <unordered_map>
#include <functional>
#include <memory>
#include "IComponent.h"


namespace TDEngine2
{
	class IComponent;
	class IComponentFactory;


	TDE2_DECLARE_SCOPED_PTR(IComponentFactory)


	/*!
		class CComponentManager

		\brief The class represents a component manager, which
		creates, destroys and stores all components in the engine
	*/

	class CComponentManager : public CBaseObject, public IComponentManager
	{
		public:
			friend TDE2_API IComponentManager* CreateComponentManager(E_RESULT_CODE& result);
		protected:
			typedef std::unordered_map<TypeId, std::unordered_map<TEntityId, U32>> TComponentEntityMap;

			typedef std::unordered_map<TEntityId, std::unordered_map<TypeId, U32>> TEntityComponentMap;

			typedef std::unordered_map<TypeId, U32>                                TComponentHashTable;

			typedef std::unordered_map<TypeId, U32>                                TComponentFactoriesMap;

			typedef std::vector<TPtr<IComponentFactory>>                           TComponentFactoriesArray;

			typedef std::list<U32>                                                 TFreeEntitiesRegistry;

			typedef std::vector<std::vector<IComponent*>>                          TComponentsMatrix;

			typedef std::unordered_map<TypeId, TEntityId>                          TUniqueComponentsTable;
		public:
			/*!
				\brief The method initializes a component manager's instance

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init() override;

			/*!
				\brief The method registers specified resource factory within a manager

				\param[in] pFactory A pointer to IComponentFactory's implementation

				\return The method returns an object, which contains a status of method's execution and
				an identifier of the registred factory
			*/

			TDE2_API E_RESULT_CODE RegisterFactory(TPtr<IComponentFactory> pFactory) override;

			TDE2_API E_RESULT_CODE UnregisterFactory(TypeId typeId) override;

			TDE2_API IComponent* CreateComponent(TEntityId entityId, TypeId componentTypeId) override;

			TDE2_API E_RESULT_CODE RemoveComponent(TypeId componentTypeId, TEntityId entityId) override;

			/*!
				\brief The method removes all components that are related with the entity.
				This method doesn't free the memory that is occupied by components.

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE RemoveComponents(TEntityId id) override;

			/*!
			\brief The method removes all components that are related with the entity and
			frees the memory occupied by them.

			\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE RemoveComponentsImmediately(TEntityId id) override;

			/*!
				\return The method returns an array of components that belong to given entity
			*/

			TDE2_API std::vector<IComponent*> GetComponents(TEntityId id) const override;

			TDE2_API bool HasComponent(TEntityId id, TypeId componentTypeId) override;

			/*!
				\brief The method returns a one way iterator to an array of components of specified type

				\param[in] typeId A type of a component

				\return The method returns a one way iterator to an array of components of specified type
			*/

			TDE2_API CComponentIterator FindComponentsOfType(TypeId typeId) override;

			/*!
				\brief The method iterates over each entity, which has specified component

				\param[in] componentTypeId A type of a component

				\param[in] action A callback that will be executed for each entity
			*/

			TDE2_API void ForEach(TypeId componentTypeId, const std::function<void(TEntityId entityId, IComponent* pComponent)>& action) override;

			TDE2_API void ForEachFactory(const std::function<void(TPtr<IComponentFactory>)>& action = nullptr) override;

			/*!
				\brief The method returns an array of entities identifiers, which have all of
				specified components

				\param[in] types An array that contains types identifiers that an entity should have

				\return The method returns an array of entities identifiers, which have all of
				specified components
			*/

			TDE2_API std::vector<TEntityId> FindEntitiesWithAll(const std::vector<TypeId>& types) override;

			/*!
				\brief The method returns an array of entities identifiers, which have any of
				specified components

				\param[in] types An array that contains types identifiers that an entity should have

				\return The method returns an array of entities identifiers, which have any of
				specified components
			*/

			TDE2_API std::vector<TEntityId> FindEntitiesWithAny(const std::vector<TypeId>& types) override;

			/*!
				\param[in] types An array that contains types identifiers that an entity should have. Note that the method
				isn't responsible for creating a new instances of unqiue components.

				\return The method returns an entity which holds a unique component
			*/

			TDE2_API TEntityId FindEntityWithUniqueComponent(TypeId typeId) override;

			TDE2_API static E_RESULT_CODE RegisterUniqueComponentType(TypeId typeId);

#if TDE2_EDITORS_ENABLED
			TDE2_API const std::vector<TComponentTypeInfo>& GetRegisteredComponentsIdentifiers() const override;
#endif
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CComponentManager)

			TDE2_API IComponent* _createComponent(TypeId componentTypeId, TEntityId entityId) override;

			TDE2_API IComponent* _getComponent(TypeId componentTypeId, TEntityId entityId) override;

			TDE2_API E_RESULT_CODE _removeComponentImmediately(TypeId componentTypeId, TEntityId entityId) override;

			TDE2_API E_RESULT_CODE _removeComponentWithAction(TypeId componentTypeId, TEntityId entityId,
															  const std::function<E_RESULT_CODE(IComponent*&)>& action);

			TDE2_API E_RESULT_CODE _removeComponentsWithAction(TEntityId entityId, const std::function<E_RESULT_CODE(IComponent*&)>& action);
			
			TDE2_API E_RESULT_CODE _registerBuiltinComponentFactories();

			TDE2_API E_RESULT_CODE _unregisterBuiltinComponentFactories();

			TDE2_API bool _hasComponent(TypeId componentTypeId, TEntityId entityId) override;
			TDE2_API bool _isUniqueComponent(TypeId componentTypeId) const;
			
			TDE2_API E_RESULT_CODE _onFreeInternal() override;
		protected:
			static std::unordered_set<TypeId> mUniqueComponentTypesRegistry;

			TComponentEntityMap      mComponentEntityMap;

			TEntityComponentMap      mEntityComponentMap;

			TComponentHashTable      mComponentsHashTable;

			TComponentsMatrix        mActiveComponents;

			std::list<IComponent*>   mDestroyedComponents;

			TComponentFactoriesMap   mComponentFactoriesMap;

			TComponentFactoriesArray mComponentFactories; 
			
			TFreeEntitiesRegistry    mFreeComponentFactoriesRegistry;

			TUniqueComponentsTable   mUniqueComponentsRegistry;

#if TDE2_EDITORS_ENABLED
			std::vector<TComponentTypeInfo> mRegisteredComponentTypes;
#endif
	};


	/*!
		\brief A factory function for creation objects of CComponentManager's type.

		\return A pointer to IComponentManager's implementation
	*/

	TDE2_API IComponentManager* CreateComponentManager(E_RESULT_CODE& result);


	/*!
		\brief The struct is used to register unique components at program's start up.
		Use this in *.cpp files only to prevent pollution of a global static scope
	*/

	template <typename TComponentType>
	struct TUniqueComponentRecorder
	{
		TUniqueComponentRecorder()
		{
			CComponentManager::RegisterUniqueComponentType(TComponentType::GetTypeId());
		}
	};


#define TDE2_REGISTER_UNIQUE_COMPONENT(TComponentType) \
	static TUniqueComponentRecorder<TComponentType> ComponentTypeRecorder_ ## TComponentType;
}
