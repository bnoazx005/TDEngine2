/*!
	\file CPrefabsRegistry.h
	\date 13.05.2022
	\authors Kasimov Ildar
*/

#pragma once


#include "IPrefabsRegistry.h"
#include "../core/CBaseObject.h"
#include <unordered_map>


namespace TDEngine2
{
	enum class TEntityId : U32;


	class IComponentManager;
	class CEntityManager;
	class IEventManager;


	TDE2_DECLARE_SCOPED_PTR(IComponentManager)
	TDE2_DECLARE_SCOPED_PTR(CEntityManager)
	TDE2_DECLARE_SCOPED_PTR(IEventManager)


	/*!
		\brief A factory function for creation objects of CStaticMeshFactory's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation
		\param[in, out] pFileSystem A pointer to IFileSystem's implementation
		\param[in, out] pWorld A pointer to IWorld's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CStaticMeshFactory's implementation
	*/

	TDE2_API IPrefabsRegistry* CreatePrefabsRegistry(IResourceManager* pResourceManager, IFileSystem* pFileSystem, IWorld* pWorld, E_RESULT_CODE& result);


	/*!
		class CPrefabsRegistry
	 
		\brief The class is an implementation of a prefabs factory
	*/

	class CPrefabsRegistry : public CBaseObject, public IPrefabsRegistry
	{
		public:
			friend TDE2_API IPrefabsRegistry* CreatePrefabsRegistry(IResourceManager*, IFileSystem*, IWorld*, E_RESULT_CODE&);
		public:
			struct TPrefabInfoEntity
			{
				TEntityId mRootEntityId = TEntityId::Invalid;
				std::vector<TEntityId> mRelatedEntities;
			};

			typedef std::unordered_map<std::string, TPrefabInfoEntity> TPrefabsTable;
		public:
			/*!
				\brief The method initializes the internal state of the object

				\param[in, out] pResourceManager A pointer to IResourceManager implementation
				\param[in, out] pFileSystem A pointer to IFileSystem implementation
				\param[in, out] pWorld A pointer to IWorld implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IFileSystem* pFileSystem, IWorld* pWorld) override;

			TDE2_API CEntity* Spawn(const std::string& id, CEntity* pParent = nullptr, const TEntityCallback & prefabEntityVisitor = nullptr) override;

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
			TPtr<IComponentManager> mpComponentsManager;
			TPtr<CEntityManager>    mpEntitiesManager;
			TPtr<IEventManager>     mpProxyEventsManager;

			TPrefabsTable           mPrefabsToEntityTable;
	};
}