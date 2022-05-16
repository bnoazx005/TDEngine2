/*!
	\file CWorld.h
	\date 29.09.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "../core/CBaseObject.h"
#include "../core/Event.h"
#include "IWorld.h"
#include <mutex>


namespace TDEngine2
{
	class ISystem;
	class CEntity;
	class CEntityManager;
	class IComponentManager;
	class ISystemManager;
	class IComponentIterator;


	/*!
		\brief A factory function for creation objects of CWorld's type.

		\param[in, out] pEventManager A pointer to IEventManager implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CWorld's implementation
	*/

	TDE2_API IWorld* CreateWorld(TPtr<IEventManager> pEventManager, E_RESULT_CODE& result);


	/*!
		interface CWorld

		\brief The class is a main scene object, which
		manages all existing entities and components
	*/

	class CWorld : public CBaseObject, public IWorld
	{
		public:
			friend TDE2_API IWorld* CreateWorld(TPtr<IEventManager>, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes a world's instance

				\param[in, out] A pointer to IEventManager implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(TPtr<IEventManager> pEventManager) override;
			
			/*!
				\brief The method creates a new instance of CEntity

				\return A pointer to a new instance of CEntity, nullptr may be returned
			*/

			TDE2_API CEntity* CreateEntity() override;

			/*!
				\brief The method creates a new instance of CEntity

				\param[in] name A name of an entity

				\return A pointer to a new instance of CEntity, nullptr may be returned
			*/

			TDE2_API CEntity* CreateEntity(const std::string& name) override;

			/*!
				\brief The method destroys specified entity.
				Note that the entity won't be deleted, it will be
				reused later, so a pointer will be valid.

				\param[in] pEntity A pointer to an entity

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Destroy(CEntity* pEntity) override;

			/*!
				\brief The method destroys specified entity
				and frees the memory, that it occupies

				\param[in] pEntity A pointer to an entity

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE DestroyImmediately(CEntity* pEntity) override;

			/*!
				\brief The method registers specified system

				\param[in] A pointer to ISystem's implementation

				\param[in] priority A value that represents a priority of a system. Than higher
				priority value then sooner a system will be executed

				\return Either registered system's identifier or an error code
			*/

			TDE2_API TResult<TSystemId> RegisterSystem(ISystem* pSystem, E_SYSTEM_PRIORITY priority = E_SYSTEM_PRIORITY::SP_NORMAL_PRIORITY) override;

			/*!
				\brief The method unregisters specified system, but doesn't free its memory

				\param[in] systemId A system's identifier

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE UnregisterSystem(TSystemId systemId) override;

			/*!
				\brief The method unregisters specified system and free its memory

				\param[in] systemId A system's identifier

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE UnregisterSystemImmediately(TSystemId systemId) override;

			/*!
				\brief The method marks specified system as an active

				\param[in] systemId A system's identifier

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE ActivateSystem(TSystemId systemId) override;

			/*!
				\brief The method deactivates specified system

				\param[in] systemId A system's identifier

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE DeactivateSystem(TSystemId systemId) override;

			/*!
				\brief The method implements the logic that should be done before the object will be released
				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE OnBeforeFree() override;

			/*!
				\brief The method registers given raycasting context within the world's instance

				\param[in, out] pRaycastContext A pointer to IRaycastContext implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE RegisterRaycastContext(TPtr<IRaycastContext> pRaycastContext) override;

			/*!
				\brief The method seeks out an entity and either return it or return nullptr

				\param[in] entityId Unique entity's identifier

				\return The method seeks out an entity and either return it or return nullptr
			*/

			TDE2_API CEntity* FindEntity(TEntityId entityId) const override;

			/*!
				\brief The method allows to iterate over all registered systems including deactivated ones
			*/

			TDE2_API void ForEachSystem(const std::function<void(TSystemId, const ISystem* const)> action = nullptr) const override;

			/*!
				\brief The method sets up time scale factor which impacts on update cycles of all entities and systems

				\param[in] scaleFactor Could be positive, negative and zero value. The latter means some kind of pause
				for the world
			*/

			TDE2_API void SetTimeScaleFactor(F32 scaleFactor) override;

			/*!
				\brief The main method that should be implemented in all derived classes.
				It contains all the logic that the system will execute during engine's work.

				\param[in] dt A delta time's value
			*/

			TDE2_API void Update(F32 dt) override;

			/*!
				\brief The method returns a pointer to IRaycastContext, use

				\return The method returns a pointer to IRaycastContext
			*/

			TDE2_API TPtr<IRaycastContext> GetRaycastContext() const override;

			TDE2_API CEntityManager* GetEntityManager() const override;

			TDE2_API F32 GetTimeScaleFactor() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CWorld)

			TDE2_API CComponentIterator _findComponentsOfType(TypeId typeId) override;

			TDE2_API void _forEach(TypeId componentTypeId, const std::function<void(TEntityId entityId, IComponent* pComponent)>& action) override;

			TDE2_API std::vector<TEntityId> _findEntitiesWithComponents(const std::vector<TypeId>& types) override;

			TDE2_API std::vector<TEntityId> _findEntitiesWithAnyComponents(const std::vector<TypeId>& types) override;

			TDE2_API TSystemId _findSystem(TypeId typeId) override;

			TDE2_API TEntityId _findEntityWithUniqueComponent(TypeId typeId) override;

			TDE2_API E_RESULT_CODE _onFreeInternal() override;
		protected:
			CEntityManager*       mpEntityManager;

			IComponentManager*    mpComponentManager;

			ISystemManager*       mpSystemManager;

			TPtr<IEventManager>   mpEventManager;

			TPtr<IRaycastContext> mpRaycastContext;

			F32                   mTimeScaleFactor;

			mutable std::mutex    mMutex;
	};


	/*!
		struct TOnNewWorldInstanceCreated

		\brief The structure represents an event which occurs when a new instance of IWorld is created
	*/

	typedef struct TOnNewWorldInstanceCreated : TBaseEvent
	{
		TDE2_EVENT(TOnNewWorldInstanceCreated);

		TPtr<IWorld> mpWorldInstance;
	} TOnNewWorldInstanceCreated, *TOnNewWorldInstanceCreatedPtr;


	/*!
		\brief The method attaches given childEntity to parentEntity. It's basically, wrapper
		around two calls of ITransform component SetParent and AttachChild

		\param[in, out] pWorld A pointer to IWorld implementation
		\param[in] parentEntity An identifier of parent entity, couldn't be same as childEntity, but invalid value is allowed to dettach entity from current parent
		\param[in] childEntity An identifier of child that will be attached to parent entity. Couldn't be invalid or same as parent

		\return RC_OK if everything went ok, or some other code, which describes an error
	*/

	TDE2_API E_RESULT_CODE GroupEntities(IWorld* pWorld, TEntityId parentEntity, TEntityId childEntity);

	TDE2_API class ICamera* GetCurrentActiveCamera(IWorld* pWorld);
}