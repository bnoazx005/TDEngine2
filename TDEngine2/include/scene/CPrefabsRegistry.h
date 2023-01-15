/*!
	\file CPrefabsRegistry.h
	\date 13.05.2022
	\authors Kasimov Ildar
*/

#pragma once


#include "IPrefabsRegistry.h"
#include "../core/CBaseObject.h"
#include "../core/Event.h"
#include <unordered_map>
#include <functional>


namespace TDEngine2
{
	class IComponentManager;
	class IEventManager;


	TDE2_DECLARE_SCOPED_PTR(IComponentManager)
	TDE2_DECLARE_SCOPED_PTR(IEventManager)


	/*!
		\brief A factory function for creation objects of CStaticMeshFactory's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation
		\param[in, out] pFileSystem A pointer to IFileSystem's implementation
		\param[in, out] pWorld A pointer to IWorld's implementation
				\param[in, out] pEventManager A pointer to IEventManager implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CStaticMeshFactory's implementation
	*/

	TDE2_API IPrefabsRegistry* CreatePrefabsRegistry(IResourceManager* pResourceManager, IFileSystem* pFileSystem, IWorld* pWorld, IEventManager* pEventManager, E_RESULT_CODE& result);


	/*!
		class CPrefabsRegistry
	 
		\brief The class is an implementation of a prefabs factory
	*/

	class CPrefabsRegistry : public CBaseObject, public IPrefabsRegistry, public IEventHandler
	{
		public:
			friend TDE2_API IPrefabsRegistry* CreatePrefabsRegistry(IResourceManager*, IFileSystem*, IWorld*, IEventManager*, E_RESULT_CODE&);
		public:
			typedef std::unordered_map<std::string, TPrefabInfoEntity> TPrefabsTable;
		public:
			TDE2_REGISTER_TYPE(CPrefabsRegistry);

			/*!
				\brief The method initializes the internal state of the object

				\param[in, out] pResourceManager A pointer to IResourceManager implementation
				\param[in, out] pFileSystem A pointer to IFileSystem implementation
				\param[in, out] pWorld A pointer to IWorld implementation
				\param[in, out] pEventManager A pointer to IEventManager implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IFileSystem* pFileSystem, IWorld* pWorld, IEventManager* pEventManager) override;

			TDE2_API CEntity* Spawn(const std::string& id, CEntity* pParent = nullptr, const TEntityCallback & prefabEntityVisitor = nullptr) override;

#if TDE2_EDITORS_ENABLED
			/*!
				\brief The method saves the given hierarchy at specified file path

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SavePrefab(const std::string& id, const std::string& filePath, CEntity* pHierarchyRoot) override;

			/*!
				\brief The method is a part of SavePrefab method and can be used to serialize entities hierarchy into the archive

				\param[in, out] pWriter A pointer to an archive object
				\param[in] pWorld A pointer to the game world
				\param[in] pEntity A root of the hierarchy that should be serialized

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SavePrefabHierarchy(IArchiveWriter* pWriter, IWorld* pWorld, CEntity* pEntity) override;

#endif
			/*!
				\brief The method loads prefab's data from the given archive reader. The method is public but private and should
				not be used in user's runtime code
			*/

			TDE2_API TPrefabInfoEntity LoadPrefabHierarchy(IArchiveReader* pReader, CEntityManager* pEntityManager, 
				const TEntityFactoryFunctor& entityCustomFactory = nullptr,
				const TPrefabFactoryFunctor& prefabCustomFactory = nullptr) override;

			/*!
				\brief The method receives a given event and processes it

				\param[in] pEvent A pointer to event data

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE OnEvent(const TBaseEvent* pEvent) override;

			/*!
				\brief The method returns an identifier of a listener

				\return The method returns an identifier of a listener
			*/

			TDE2_API TEventListenerId GetListenerId() const override;

			/*!
				\brief The method returns an array of prefabs identifier that were declared in already loaded prefabs manifest
			*/

			TDE2_API const std::vector<std::string>& GetKnownPrefabsIdentifiers() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CPrefabsRegistry)

			TDE2_API E_RESULT_CODE _onFreeInternal() override;
		private:
			IResourceManager*       mpResourceManager;
			IFileSystem*            mpFileSystem;
			IWorld*                 mpWorld;
			IEventManager*          mpMainEventManager;
			TPtr<IComponentManager> mpComponentsManager;
			TPtr<CEntityManager>    mpEntitiesManager;
			TPtr<IEventManager>     mpProxyEventsManager;

			TPrefabsTable           mPrefabsToEntityTable;
	};
}