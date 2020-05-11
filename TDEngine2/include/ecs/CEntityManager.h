/*!
	\file CEntityManager.h
	\date 28.09.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../core/CBaseObject.h"
#include "./../utils/Utils.h"
#include "IComponentManager.h"
#include <vector>
#include <list>
#include <string>
#include <unordered_map>


namespace TDEngine2
{
	class CEntity;
	class IEventManager;
	class CEntityManager;


	/*!
		\brief A factory function for creation objects of CEntityManager's type.

		\param[in, out] pEventManager A pointer to IEventManager implementation

		\param[in, out] pComponentManager A pointer to IComponentManager implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CEntityManager's implementation
	*/

	TDE2_API CEntityManager* CreateEntityManager(IEventManager* pEventManager, IComponentManager* pComponentManager, E_RESULT_CODE& result);


	/*!
		class CEntityManager
		
		\brief The class is an entity manager that responsible for
		creation, destruction of entities. Also it implements main
		methods of CEntity such as AddComponent, GetComponent and
		RemoveComponent.
	*/

	class CEntityManager: public CBaseObject
	{
		public:
			friend TDE2_API CEntityManager* CreateEntityManager(IEventManager* pEventManager, IComponentManager* pComponentManager, E_RESULT_CODE& result);
		protected:
			typedef std::unordered_map<TEntityId, U32> TEntitiesHashTable;
		public:
			/*!
				\brief The method initializes an entity manager's instance

				\param[in, out] pEventManager A pointer to IEventManager implementation

				\param[in, out] pComponentManager A pointer to a component manager's instance

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IEventManager* pEventManager, IComponentManager* pComponentManager);

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method creates a new instance of CEntity

				\return A pointer to a new instance of CEntity, nullptr may be returned
			*/

			TDE2_API CEntity* Create();

			/*!
				\brief The method creates a new instance of CEntity

				\param[in] name A name of an entity

				\return A pointer to a new instance of CEntity, nullptr may be returned
			*/

			TDE2_API CEntity* Create(const std::string& name);

			/*!
				\brief The method destroys specified entity.
				Note that the entity won't be deleted, it will be
				reused later, so a pointer will be valid.
				
				\param[in] pEntity A pointer to an entity 

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Destroy(CEntity* pEntity);

			/*!
				\brief The method destroys specified entity
				and frees the memory, that it occupies

				\param[in] pEntity A pointer to an entity

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE DestroyImmediately(CEntity* pEntity);

			/*!
				\brief The method destroys all created entities, but
				at the same moment a piece of memory won't be freed.
				if you want to free memory immediately use
				DestroyAllImmediately method

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE DestroyAllEntities();

			/*!
				\brief The method destroys all created entities and
				frees memory they occupy

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE DestroyAllImmediately();

			/*!
				\brief The method creates a new component and connects it with
				the entity

				\return A pointer to a component, or nullptr if some error has occured
			*/

			template <typename T>
			TDE2_API T* AddComponent(TEntityId id);

			/*!
				\brief The method remove a component of specified T type

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			template <typename T>
			TDE2_API E_RESULT_CODE RemoveComponent(TEntityId id);

			/*!
				\brief The method removes all components that are related with the entity

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE RemoveComponents(TEntityId id);

			/*!
				\brief The method returns a pointer to a component of specified type T

				\return The method returns a pointer to a component of specified type T, or nullptr if there is no
				attached component of desired type.
			*/

			template <typename T>
			TDE2_API T* GetComponent(TEntityId id);
			
			/*!
				\brief The method checks up whether a given entity has a component of specified type or not

				\return The method returns true if the entity has specified component and false in other cases
			*/

			template <typename T>
			TDE2_API bool HasComponent(TEntityId id);

			/*!
				\brief The method seeks out an entity and either return it or return nullptr

				\param[in] entityId Unique entity's identifier

				\return The method seeks out an entity and either return it or return nullptr
			*/

			TDE2_API CEntity* GetEntity(TEntityId entityId) const;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CEntityManager)

			std::string _constructDefaultEntityName(U32 id) const;

			TDE2_API CEntity* _createEntity(const std::string& name);

			TDE2_API void _notifyOnAddComponent(TEntityId entityId, TComponentTypeId componentTypeId);

			TDE2_API void _notifyOnRemovedComponent(TEntityId entityId, TComponentTypeId componentTypeId);
		protected:
			std::vector<CEntity*> mActiveEntities;

			std::list<CEntity*>   mDestroyedEntities;

			TEntitiesHashTable    mEntitiesHashTable;

			U32                   mNextIdValue;

			IComponentManager*    mpComponentManager;

			IEventManager*        mpEventManager;
	};
	

	template <typename T>
	TDE2_API T* CEntityManager::AddComponent(TEntityId id)
	{
		T* componentInstance = mpComponentManager->CreateComponent<T>(id);

		_notifyOnAddComponent(id, T::GetTypeId());

		return componentInstance;
	}

	template <typename T>
	TDE2_API E_RESULT_CODE CEntityManager::RemoveComponent(TEntityId id)
	{
		E_RESULT_CODE result = mpComponentManager->RemoveComponent<T>(id);

		if (result != RC_OK)
		{
			return result;
		}

		_notifyOnRemovedComponent(id, T::GetTypeId());

		return result;
	}

	template <typename T>
	TDE2_API T* CEntityManager::GetComponent(TEntityId id)
	{
		return mpComponentManager->GetComponent<T>(id);
	}
	
	template <typename T>
	TDE2_API bool CEntityManager::HasComponent(TEntityId id)
	{
		return mpComponentManager->HasComponent<T>(id);
	}
}
