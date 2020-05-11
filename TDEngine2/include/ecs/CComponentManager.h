/*!
	\file CComponentManager.h
	\date 28.09.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../core/CBaseObject.h"
#include "./../ecs/IComponentManager.h"
#include <vector>
#include <list>
#include <unordered_map>
#include <functional>
#include <memory>
#include "./../utils/Utils.h"
#include "IComponent.h"


namespace TDEngine2
{
	class IComponent;
	class IComponentFactory;


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

			typedef std::vector<const IComponentFactory*>                          TComponentFactoriesArray;

			typedef std::list<U32>                                                 TFreeEntitiesRegistry;

			typedef std::vector<std::vector<IComponent*>>                          TComponentsMatrix;
		public:
			/*!
				\brief The method initializes a component manager's instance

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init() override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method registers specified resource factory within a manager

				\param[in] pFactory A pointer to IComponentFactory's implementation

				\return The method returns an object, which contains a status of method's execution and
				an identifier of the registred factory
			*/

			TDE2_API E_RESULT_CODE RegisterFactory(const IComponentFactory* pFactory) override;

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
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CComponentManager)

			TDE2_API E_RESULT_CODE _unregisterFactory(TypeId typeId) override;

			TDE2_API IComponent* _createComponent(TypeId componentTypeId, TEntityId entityId) override;

			TDE2_API IComponent* _getComponent(TypeId componentTypeId, TEntityId entityId) override;

			TDE2_API E_RESULT_CODE _removeComponent(TypeId componentTypeId, TEntityId entityId) override;

			TDE2_API E_RESULT_CODE _removeComponentImmediately(TypeId componentTypeId, TEntityId entityId) override;

			TDE2_API E_RESULT_CODE _removeComponentWithAction(TypeId componentTypeId, TEntityId entityId,
															  const std::function<E_RESULT_CODE(IComponent*&)>& action);

			TDE2_API E_RESULT_CODE _removeComponentsWithAction(TEntityId entityId, const std::function<E_RESULT_CODE(IComponent*&)>& action);
			
			TDE2_API E_RESULT_CODE _registerBuiltinComponentFactories();

			TDE2_API E_RESULT_CODE _unregisterBuiltinComponentFactories();

			TDE2_API bool _hasComponent(TypeId componentTypeId, TEntityId entityId) override;
		protected:
			TComponentEntityMap      mComponentEntityMap;

			TEntityComponentMap      mEntityComponentMap;

			TComponentHashTable      mComponentsHashTable;

			TComponentsMatrix        mActiveComponents;

			std::list<IComponent*>   mDestroyedComponents;

			TComponentFactoriesMap   mComponentFactoriesMap;

			TComponentFactoriesArray mComponentFactories; 
			
			TFreeEntitiesRegistry    mFreeComponentFactoriesRegistry;
	};


	/*!
		\brief A factory function for creation objects of CComponentManager's type.

		\return A pointer to IComponentManager's implementation
	*/

	TDE2_API IComponentManager* CreateComponentManager(E_RESULT_CODE& result);
}
