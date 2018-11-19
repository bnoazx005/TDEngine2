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
#include "./../utils/Utils.h"


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
			typedef std::unordered_map<TComponentTypeId, std::unordered_map<TEntityId, U32>> TComponentEntityMap;
			typedef std::unordered_map<TEntityId, std::unordered_map<TComponentTypeId, U32>> TEntityComponentMap;
			
			constexpr static U32 mInvalidMapValue = 0;

			typedef std::unordered_map<TypeId, U32>       TComponentFactoriesMap;

			typedef std::vector<const IComponentFactory*> TComponentFactoriesArray;

			typedef std::list<U32>                        TFreeEntitiesRegistry;
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
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CComponentManager)

			TDE2_API E_RESULT_CODE _unregisterFactory(TypeId typeId) override;

			TDE2_API IComponent* _createComponent(TypeId componentTypeId, TEntityId entityId) override;

			TDE2_API IComponent* _getComponent(TypeId componentTypeId, TEntityId entityId) override;

			TDE2_API E_RESULT_CODE _removeComponent(TypeId componentTypeId, TEntityId entityId) override;

			TDE2_API E_RESULT_CODE _removeComponentImmediately(TypeId componentTypeId, TEntityId entityId) override;

			TDE2_API E_RESULT_CODE _registerBuiltinComponentFactories();

			TDE2_API E_RESULT_CODE _unregisterBuiltinComponentFactories();
		protected:
			TComponentEntityMap      mComponentEntityMap;
			TEntityComponentMap      mEntityComponentMap;
			std::vector<IComponent*> mActiveComponents;
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
