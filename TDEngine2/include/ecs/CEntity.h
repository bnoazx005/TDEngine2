/*!
	\file CEntity.h
	\date 28.09.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../core/CBaseObject.h"
#include "./../utils/Utils.h"
#include "CEntityManager.h"


namespace TDEngine2
{	
	class CEntityManager;

	/*!
		class CEntity

		\brief The class is a basic thing within a scene.
		Basically, it's just a handler to group a bunch of
		components.
	*/

	class CEntity : public CBaseObject
	{
		public:
			friend TDE2_API CEntity* CreateEntity(TEntityId id, CEntityManager* pEntityManager, E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an entity

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(TEntityId id, CEntityManager* pEntityManager);

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The methods set up an identifier for an entity.
				Note that the id should be unique for a whole scene.

				\param[in] id An identifier's value
			*/

			TDE2_API virtual void SetId(TEntityId id);

			/*!
				\brief The method resets current state of an entity
				to default one
			*/

			TDE2_API virtual void Reset();

			/*!
				\brief The method creates a new component and connects it with
				the entity

				\return A pointer to a component, or nullptr if some error has occured
			*/

			template <typename T>
			TDE2_API T* AddComponent();

			/*!
				\brief The method remove a component of specified T type

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			template <typename T>
			TDE2_API E_RESULT_CODE RemoveComponent();

			/*!
				\brief The method removes all components that are related with the entity

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE RemoveComponents();

			/*!
				\brief The method returns a pointer to a component of specified type T

				\return The method returns a pointer to a component of specified type T, or nullptr if there is no
				attached component of desired type.
			*/

			template <typename T>
			TDE2_API T* GetComponent();

			/*!
				\brief The method returns an entity's id

				\return The method returns an entity's id
			*/

			TDE2_API virtual TEntityId GetId() const;

			TDE2_API virtual bool operator== (const CEntity* pEntity) const;

			TDE2_API virtual bool operator!= (const CEntity* pEntity) const;

			TDE2_API virtual bool operator== (TEntityId otherId) const;

			TDE2_API virtual bool operator!= (TEntityId otherId) const;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CEntity)
		protected:
			CEntityManager* mpEntityManager;

			TEntityId       mId;
	};


	template <typename T>
	TDE2_API T* CEntity::AddComponent()
	{
		return mpEntityManager->AddComponent<T>(mId);
	}

	template <typename T>
	TDE2_API E_RESULT_CODE CEntity::RemoveComponent()
	{
		return mpEntityManager->RemoveComponent<T>(mId);
	}

	template <typename T>
	TDE2_API T* CEntity::GetComponent()
	{
		return mpEntityManager->GetComponent<T>(mId);
	}


	/*!
		\brief A factory function for creation objects of CEntity's type.

		\return A pointer to CEntity's implementation
	*/

	TDE2_API CEntity* CreateEntity(TEntityId id, CEntityManager* pEntityManager, E_RESULT_CODE& result);
}
