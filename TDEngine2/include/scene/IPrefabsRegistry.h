/*!
	\file IPrefabsRegistry.h
	\date 13.05.2022
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Utils.h"
#include "../utils/Types.h"
#include "../core/IBaseObject.h"
#include <functional>


namespace TDEngine2
{
	class IFileSystem;
	class IResourceManager;
	class IWorld;
	class CEntity;
	class IEventManager;


	TDE2_DECLARE_SCOPED_PTR(IFileSystem)
	TDE2_DECLARE_SCOPED_PTR(IResourceManager)
	TDE2_DECLARE_SCOPED_PTR(IWorld)


	/*!
		interface IPrefabsRegistry

		\brief The interface describes functionality of prefabs collection. It lazy loads prefabs and reuse this information later.
		
		The manifest of prefabs has the following structure

		prefabs:
			- prefab-id: PrefabId 
				physical-path-to-prefab: /Prefabs/Prefab1
			...

		It's implemented in the engine as a resource

	*/

	class IPrefabsRegistry: public virtual IBaseObject
	{
		public:
			typedef std::function<void(TEntityId)> TEntityCallback;
		public:
			/*!
				\brief The method initializes the internal state of the object

				\param[in, out] pResourceManager A pointer to IResourceManager implementation
				\param[in, out] pFileSystem A pointer to IFileSystem implementation
				\param[in, out] pWorld A pointer to IWorld implementation
				\param[in, out] pEventManager A pointer to IEventManager implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IFileSystem* pFileSystem, IWorld* pWorld, IEventManager* pEventManager) = 0;

			/*!
				\brief The method instantiates a new copy of specified prefab's hierarchy

				\param[in] id An identifier of a prefab which is defined in prefabs collection
				\param[in, out] pParent A pointer which the new instantiated tree will be attached to
				\param[in] prefabEntityVisitor A callback that's invoked over each created entity of prefab's isntance

				\return A pointer to a root entity of a prefab's instance
			*/

			TDE2_API virtual CEntity* Spawn(const std::string& id, CEntity* pParent = nullptr, const TEntityCallback& prefabEntityVisitor = nullptr) = 0;

#if TDE2_EDITORS_ENABLED
			/*!
				\brief The method saves the given hierarchy at specified file path

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE SavePrefab(const std::string& id, const std::string& filePath, CEntity* pHierarchyRoot) = 0;
#endif

			/*!
				\brief The method returns an array of prefabs identifier that were declared in already loaded prefabs manifest 
			*/

			TDE2_API virtual const std::vector<std::string>& GetKnownPrefabsIdentifiers() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IPrefabsRegistry)
	};


	TDE2_DECLARE_SCOPED_PTR(IPrefabsRegistry)
}