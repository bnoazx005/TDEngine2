/*!
	\file CEntity.h
	\date 28.09.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "../core/CBaseObject.h"
#include "../core/Event.h"
#include "../core/memory/CPoolAllocator.h"
#include "../utils/Utils.h"
#include "../core/Serialization.h"
#include "CEntityManager.h"
#include <string>


namespace TDEngine2
{	
	class CEntityManager;
	class IComponent;
	class IWorld;

	struct TEntitiesMapper;

	/*!
		class CEntity

		\brief The class is a basic thing within a scene.
		Basically, it's just a handler to group a bunch of
		components.
	*/

	class CEntity : public CBaseObject, public ISerializable, public CPoolMemoryAllocPolicy<CEntity, 1 << 20>, public INonAllocCloneable<CEntity>
	{
		public:
			friend TDE2_API CEntity* CreateEntity(TEntityId id, const std::string& name, CEntityManager* pEntityManager, E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an entity

				\param[in] id Unique identifier of an entity

				\param[in] name A name of an entity

				\param[in, out] pEntityManager A pointer to CEntityManager class

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(TEntityId id, const std::string& name, CEntityManager* pEntityManager);

			/*!
				\brief The method deserializes object's state from given reader

				\param[in, out] pReader An input stream of data that contains information about the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Load(IArchiveReader* pReader) override;

			/*!
				\brief The method serializes object's state into given stream

				\param[in, out] pWriter An output stream of data that writes information about the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Save(IArchiveWriter* pWriter) override;

			/*!
				\brief The method is called after all entities of particular scene were loaded. It remaps all identifiers to
				make them correctly corresponds to saved state
			*/

			TDE2_API E_RESULT_CODE PostLoad(CEntityManager* pEntityManager, const TEntitiesMapper& entitiesIdentifiersRemapper);

			/*!
				\brief The method creates a new deep copy of the instance and returns a smart pointer to it.
				The original state of the object stays the same

				\param[in] pDestObject A valid pointer to an object which the properties will be assigned into
			*/

			TDE2_API E_RESULT_CODE Clone(CEntity*& pDestObject) const override;

			/*!
				\brief The methods set up an identifier for an entity.
				Note that the id should be unique for a whole scene.

				\param[in] id An identifier's value
			*/

			TDE2_API virtual void SetId(TEntityId id);

			/*!
				\brief The method sets up a name of an entity

				\param[in] name A new name of the entity
			*/

			TDE2_API virtual void SetName(const std::string& name);

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

			TDE2_API IComponent* AddComponent(TypeId componentTypeId);

			/*!
				\brief The method remove a component of specified T type

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			template <typename T>
			TDE2_API E_RESULT_CODE RemoveComponent();

			TDE2_API E_RESULT_CODE RemoveComponent(TypeId componentTypeId);

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
				\return The method returns an array of all components that're related with the corresponding entity
			*/

			TDE2_API std::vector<IComponent*> GetComponents() const;

			/*!
				\brief The method checks up whether the entity has corresponding component or not

				\return The method returns true if the entity has specified component and false in other cases
			*/

			template <typename T>
			TDE2_API bool HasComponent();

			TDE2_API bool HasComponent(TypeId componentTypeId);

			/*!
				\brief The method returns an entity's id

				\return The method returns an entity's id
			*/

			TDE2_API virtual TEntityId GetId() const;

			/*!
				\brief The method returns a name of an entity

				\return The method returns a name of an entity
			*/

			TDE2_API virtual const std::string& GetName() const;

			TDE2_API virtual bool operator== (const CEntity* pEntity) const;

			TDE2_API virtual bool operator!= (const CEntity* pEntity) const;

			TDE2_API virtual bool operator== (TEntityId otherId) const;

			TDE2_API virtual bool operator!= (TEntityId otherId) const;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CEntity)

			TDE2_API IComponent* _addComponentInternal(TypeId typeId);
		protected:
			CEntityManager* mpEntityManager;

			TEntityId       mId;

			std::string     mName;
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

	template <typename T>
	TDE2_API bool CEntity::HasComponent()
	{
		return mpEntityManager->HasComponent<T>(mId);
	}


	/*!
		\brief A factory function for creation objects of CEntity's type.

		\return A pointer to CEntity's implementation
	*/

	TDE2_API CEntity* CreateEntity(TEntityId id, const std::string& name, CEntityManager* pEntityManager, E_RESULT_CODE& result);


	/*!
		struct TOnEntityCreatedEvent

		\brief The structure represents an event which occurs 
		when a new entity was created
	*/

	typedef struct TOnEntityCreatedEvent: TBaseEvent
	{
		virtual ~TOnEntityCreatedEvent() = default;

		TDE2_REGISTER_TYPE(TOnEntityCreatedEvent)

		REGISTER_EVENT_TYPE(TOnEntityCreatedEvent)

		TEntityId mCreatedEntityId;
	} TOnEntityCreatedEvent, *TOnEntityCreatedEventPtr;

	
	/*!
		struct TOnEntityRemovedEvent

		\brief The structure represents an event which occurs
		when an entity was removed
	*/

	typedef struct TOnEntityRemovedEvent: TBaseEvent
	{
		virtual ~TOnEntityRemovedEvent() = default;

		TDE2_REGISTER_TYPE(TOnEntityRemovedEvent)

		REGISTER_EVENT_TYPE(TOnEntityRemovedEvent)

		TEntityId mRemovedEntityId;
	} TOnEntityRemovedEvent, *TOnEntityRemovedEventPtr;


	/*!
		\brief The type is used to resolve entities identifiers when they're deserialized
	*/

	struct TEntitiesMapper
	{
		std::unordered_map<TEntityId, TEntityId> mSerializedToRuntimeIdsTable;

		/*!
			\return The method returns resolved identifier's value if it exists in mSerializedToRuntimeIdsTable hash table. If there is no
			such pair the input identifier is returned
		*/

		TDE2_API TEntityId Resolve(TEntityId input) const;
	};
}
