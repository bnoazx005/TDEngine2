/*!
	\file CComponentManager.h
	\date 28.09.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../core/CBaseObject.h"
#include "IComponent.h"
#include <vector>
#include <list>
#include <unordered_map>


namespace TDEngine2
{

	/*!
		class CComponentManager

		\brief The class represents a component manager, which
		creates, destroys and stores all components in the engine
	*/

	class CComponentManager : public CBaseObject
	{
		public:
			friend TDE2_API CComponentManager* CreateComponentManager(E_RESULT_CODE& result);
		protected:
			typedef std::unordered_map<TComponentTypeId, std::unordered_map<TEntityId, U32>> TComponentEntityMap;
			typedef std::unordered_map<TEntityId, std::unordered_map<TComponentTypeId, U32>> TEntityComponentMap;
			
			constexpr static U32 mInvalidMapValue = 0;
		public:
			/*!
				\brief The method initializes a component manager's instance

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init();

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method creates a new component and connects it with
				the entity

				\return A pointer to a component, or nullptr if some error has occured
			*/

			template <typename T>
			TDE2_API T* CreateComponent(TEntityId id)
			{
				TComponentTypeId componentType = T::GetTypeId();

				T* pNewComponent = GetComponent<T>(id);

				if (pNewComponent)
				{
					return pNewComponent;
				}

				pNewComponent = new T();

				mActiveComponents.push_back(pNewComponent);

				size_t hash = mActiveComponents.size();

				mComponentEntityMap[componentType][id] = hash;
				mEntityComponentMap[id][componentType] = hash;

				return pNewComponent;
			}

			/*!
				\brief The method removes a component of specified T type.
				This method doesn't free the memory that is occupied by a component.

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			template <typename T>
			TDE2_API E_RESULT_CODE RemoveComponent(TEntityId id)
			{
				TComponentTypeId componentType = T::GetTypeId();

				U32 targetEntityComponentHash = mEntityComponentMap[id][componentType];

				if (targetEntityComponentHash == mInvalidMapValue)
				{
					return RC_FAIL;
				}

				IComponent* pComponent = mActiveComponents[--targetEntityComponentHash];

				if (!pComponent)
				{
					return RC_FAIL;
				}

				mActiveComponents[targetEntityComponentHash] = nullptr;

				mDestroyedComponents.push_back(pComponent);

				// mark handlers as invalid
				mEntityComponentMap[id][componentType] = 0;
				mComponentEntityMap[componentType][id] = 0;

				return RC_OK;
			}

			/*!
			\brief The method removes a component of specified T type and
			frees the memory occupied by it

			\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			template <typename T>
			TDE2_API E_RESULT_CODE RemoveComponentImmediately(TEntityId id)
			{
				TComponentTypeId componentType = T::GetTypeId();

				U32 targetEntityComponentHash = mEntityComponentMap[id][componentType];

				if (targetEntityComponentHash == mInvalidMapValue)
				{
					return RC_FAIL;
				}

				IComponent* pComponent = mActiveComponents[--targetEntityComponentHash];

				if (!pComponent)
				{
					return RC_FAIL;
				}

				mActiveComponents[targetEntityComponentHash] = nullptr;

				E_RESULT_CODE result = RC_OK;

				if ((result = pComponent->Free()) != RC_OK)
				{
					return result;
				}

				// mark handlers as invalid
				mEntityComponentMap[id][componentType] = 0;
				mComponentEntityMap[componentType][id] = 0;

				return RC_OK;
			}

			/*!
				\brief The method removes all components that are related with the entity.
				This method doesn't free the memory that is occupied by components.

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE RemoveComponents(TEntityId id);

			/*!
			\brief The method removes all components that are related with the entity and
			frees the memory occupied by them.

			\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE RemoveComponentsImmediately(TEntityId id);

			/*!
				\brief The method returns a pointer to a component of specified type T

				\return The method returns a pointer to a component of specified type T, or nullptr if there is no
				attached component of desired type.
			*/

			template <typename T>
			TDE2_API T* GetComponent(TEntityId id)
			{
				TComponentTypeId componentType = T::GetTypeId();

				U32 hashValue = mComponentEntityMap[componentType][id];

				if (hashValue == mInvalidMapValue)
				{
					return nullptr;
				}

				return dynamic_cast<T*>(mActiveComponents[hashValue - 1]);
			}
		protected:
			TDE2_API CComponentManager();
			TDE2_API virtual ~CComponentManager() = default;
			TDE2_API CComponentManager(const CComponentManager& componentManager) = delete;
			TDE2_API virtual CComponentManager& operator= (CComponentManager& componentManager) = delete;
		protected:
			TComponentEntityMap      mComponentEntityMap;
			TEntityComponentMap      mEntityComponentMap;
			std::vector<IComponent*> mActiveComponents;
			std::list<IComponent*>   mDestroyedComponents;
	};


	/*!
		\brief A factory function for creation objects of CComponentManager's type.

		\return A pointer to CComponentManager's implementation
	*/

	TDE2_API CComponentManager* CreateComponentManager(E_RESULT_CODE& result);
}
