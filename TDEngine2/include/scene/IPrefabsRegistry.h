/*!
	\file IPrefabsRegistry.h
	\date 13.05.2022
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Utils.h"
#include "../utils/Types.h"
#include "../core/IBaseObject.h"
#include "../math/TVector3.h"
#include <functional>


namespace TDEngine2
{
	enum class TEntityId : U32;


	class IFileSystem;
	class IResourceManager;
	class IWorld;
	class CEntity;
	class IEventManager;
	class IArchiveReader;
	class IArchiveWriter;
	class CEntityManager;


	TDE2_DECLARE_SCOPED_PTR(IFileSystem)
	TDE2_DECLARE_SCOPED_PTR(IResourceManager)
	TDE2_DECLARE_SCOPED_PTR(IWorld)
	TDE2_DECLARE_SCOPED_PTR(CEntityManager)
	TDE2_DECLARE_SCOPED_PTR(CEntity)


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
			typedef std::function<void(TEntityId)>                        TEntityCallback;
			typedef std::function<CEntity*(TEntityId)>                    TEntityFactoryFunctor;
			typedef std::function<CEntity*(const std::string&, CEntity*)> TPrefabFactoryFunctor;

			struct TPrefabInfoEntity
			{
				struct TPrefabLinkInfo
				{
					TVector3    mPosition = ZeroVector3;
					TEntityId   mId = TEntityId::Invalid;
					TEntityId   mParentId = TEntityId::Invalid;
					std::string mPrefabId;
				};


				TEntityId                    mRootEntityId = TEntityId::Invalid;
				std::vector<TEntityId>       mRelatedEntities;

				std::vector<TPrefabLinkInfo> mNestedPrefabsLinks;
			};
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

			/*!
				\brief The method is a part of SavePrefab method and can be used to serialize entities hierarchy into the archive

				\param[in, out] pWriter A pointer to an archive object
				\param[in] pWorld A pointer to the game world 
				\param[in] pEntity A root of the hierarchy that should be serialized

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE SavePrefabHierarchy(IArchiveWriter* pWriter, IWorld* pWorld, CEntity* pEntity) = 0;
#endif

			/*!
				\brief The method loads prefab's data from the given archive reader. The method is allowed only in editors builds
			*/

			TDE2_API virtual TPrefabInfoEntity LoadPrefabHierarchy(IArchiveReader* pReader, CEntityManager* pEntityManager, 
				const TEntityFactoryFunctor& entityCustomFactory = nullptr,
				const TPrefabFactoryFunctor& prefabCustomFactory = nullptr) = 0;

			/*!
				\brief The method returns an array of prefabs identifier that were declared in already loaded prefabs manifest 
			*/

			TDE2_API virtual const std::vector<std::string>& GetKnownPrefabsIdentifiers() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IPrefabsRegistry)
	};


	TDE2_DECLARE_SCOPED_PTR(IPrefabsRegistry)
}