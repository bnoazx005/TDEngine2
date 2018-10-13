/*!
	\file CEntityManager.h
	\date 28.09.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../core/CBaseObject.h"
#include "CComponentManager.h"
#include <vector>
#include <list>


namespace TDEngine2
{
	class CEntity;

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
			friend TDE2_API CEntityManager* CreateEntityManager(CComponentManager* pComponentManager, E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an entity manager's instance

				\param[in] pComponentManager A pointer to a component manager's instance

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(CComponentManager* pComponentManager);

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
		protected:
			TDE2_API CEntityManager();
			TDE2_API virtual ~CEntityManager() = default;
			TDE2_API CEntityManager(const CEntityManager& entityMgr) = delete;
			TDE2_API virtual CEntityManager& operator= (CEntityManager& entityMgr) = delete;
		protected:
			std::vector<CEntity*> mActiveEntities;
			std::list<CEntity*>   mDestroyedEntities;
			TEntityId             mNextIdValue;
			CComponentManager*    mpComponentManager;
	};
	

	template <typename T>
	TDE2_API T* CEntityManager::AddComponent(TEntityId id)
	{
		return mpComponentManager->CreateComponent<T>(id);
	}

	template <typename T>
	TDE2_API E_RESULT_CODE CEntityManager::RemoveComponent(TEntityId id)
	{
		return mpComponentManager->RemoveComponent<T>(id);
	}

	template <typename T>
	TDE2_API T* CEntityManager::GetComponent(TEntityId id)
	{
		return mpComponentManager->GetComponent<T>(id);
	}


	/*!
		\brief A factory function for creation objects of CEntityManager's type.

		\return A pointer to CEntityManager's implementation
	*/

	TDE2_API CEntityManager* CreateEntityManager(CComponentManager* pComponentManager, E_RESULT_CODE& result);
}
