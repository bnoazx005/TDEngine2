#include "../../include/scene/CPrefabsRegistry.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/core/CEventManager.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/ecs/CComponentManager.h"
#include "../../include/ecs/CEntityManager.h"
#include "../../include/scene/CPrefabsManifest.h"
#include "../../include/platform/CYAMLFile.h"
#include "../../include/utils/CFileLogger.h"


namespace TDEngine2
{
	CPrefabsRegistry::CPrefabsRegistry() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CPrefabsRegistry::Init(TPtr<IResourceManager> pResourceManager, TPtr<IFileSystem> pFileSystem, TPtr<IWorld> pWorld)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pResourceManager || !pFileSystem || !pWorld)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = pResourceManager;
		mpFileSystem = pFileSystem;
		mpWorld = pWorld;

		E_RESULT_CODE result = RC_OK;

		mpComponentsManager = TPtr<IComponentManager>(CreateComponentManager(result));
		if (RC_OK != result)
		{
			return result;
		}

		mpProxyEventsManager = TPtr<IEventManager>(CreateEventManager(result));
		if (RC_OK != result)
		{
			return result;
		}

		mpEntitiesManager = TPtr<CEntityManager>(CreateEntityManager(mpProxyEventsManager.Get(), mpComponentsManager.Get(), result));
		if (RC_OK != result)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}


	static CEntity* TryGetLoadedPrefabEntity(const TPtr<IWorld>& pWorld, const CPrefabsRegistry::TPrefabsTable& loadedPrefabsTable, const std::string& id)
	{
		auto it = loadedPrefabsTable.find(id);
		return (it == loadedPrefabsTable.cend()) ? nullptr : pWorld->FindEntity(it->second);
	}


	static E_RESULT_CODE LoadPrefabHierarchy(CEntity* pPrefabRootEntity, IYAMLFileReader* pReader, TPtr<CEntityManager>& pEntityManager)
	{
		E_RESULT_CODE result = RC_OK;

		std::unordered_map<TEntityId, TEntityId> entitiesIdsMap;
		std::vector<TEntityId> createdEntities;

		createdEntities.push_back(pPrefabRootEntity->GetId());

		// \note Read entities
		result = result | pReader->BeginGroup("entities");
		{
			while (pReader->HasNextItem())
			{
				CEntity* pNewEntity = pEntityManager->Create();
				createdEntities.push_back(pNewEntity->GetId());

				result = result | pReader->BeginGroup(Wrench::StringUtils::GetEmptyStr());
				{
					result = result | pReader->BeginGroup("entity");
					{
						entitiesIdsMap.emplace(static_cast<TEntityId>(pReader->GetUInt32("id")), pNewEntity->GetId());
					}
					result = result | pReader->EndGroup();

					result = result | pNewEntity->Load(pReader);
				}
				result = result | pReader->EndGroup();

				//mEntities.push_back(pNewEntity->GetId());
			}
		}
		result = result | pReader->EndGroup();

		// \note First time we resolve references within the prefab's hierarchy. The second time we do it when the instance of the prototype is created
		for (TEntityId currEntityId : createdEntities)
		{
			if (CEntity* pEntity = pEntityManager->GetEntity(currEntityId))
			{
				result = result | pEntity->PostLoad(pEntityManager.Get(), entitiesIdsMap);
				TDE2_ASSERT(RC_OK == result);
			}
		}

		return result;
	}


	static CEntity* LoadPrefabInfoFromManifest(const TPtr<IResourceManager>& pResourceManager, const TPtr<IFileSystem>& pFileSystem, 
											TPtr<CEntityManager>& pEntityManager, TPtr<IWorld>& pWorld, const std::string& id)
	{
		/// \note Iterate over all CPrefabsManifest resources and try to find the corresponding prefab's path
		std::string pathToPrefab;
		
		for (auto&& currResourceId : pResourceManager->GetResourcesListByType<CPrefabsManifest>())
		{
			if (auto pPrefabsCollection = pResourceManager->GetResource<IPrefabsManifest>(pResourceManager->GetResourceId(currResourceId)))
			{
				pathToPrefab = pPrefabsCollection->GetPathToPrefabById(id);
				if (!pathToPrefab.empty())
				{
					break;
				}
			}
		}

		if (pathToPrefab.empty())
		{
			return nullptr;
		}

		/// \note If we've found one try to read archive's data
		/// \todo Make it more dependency free and type agnostic
		if (TResult<TFileEntryId> prefabFileId = pFileSystem->Open<IYAMLFileReader>(pathToPrefab))
		{
			CEntity* pPrefabEntity = pEntityManager->Create(id); /// \note This will be a root for all entities that's stored in the file
			TDE2_ASSERT(pPrefabEntity);

			E_RESULT_CODE result = LoadPrefabHierarchy(pPrefabEntity, pFileSystem->Get<IYAMLFileReader>(prefabFileId.Get()), pEntityManager);
			TDE2_ASSERT(RC_OK == result);

			return pPrefabEntity;
		}

		return nullptr;
	}


	CEntity* CPrefabsRegistry::Spawn(const std::string& id, CEntity* pParent)
	{
		CEntity* pPrefabEntity = TryGetLoadedPrefabEntity(mpWorld, mPrefabsToEntityTable, id);
		if (!pPrefabEntity)
		{
			pPrefabEntity = LoadPrefabInfoFromManifest(mpResourceManager, mpFileSystem, mpEntitiesManager, mpWorld, id);
		}

		if (!pPrefabEntity)
		{
			LOG_ERROR(Wrench::StringUtils::Format("[CPrefabsRegistry] The prefab {0} couldn't be loaded, there is no information about it in loaded manifests", id));
			TDE2_ASSERT(false);

			return nullptr;
		}

		/// \note Clone the entity's data into a new one
		CEntity* pPrefabInstance = mpWorld->CreateEntity(id);
		TDE2_ASSERT(pPrefabInstance);

		E_RESULT_CODE result = pPrefabEntity->Clone(pPrefabInstance);
		TDE2_ASSERT(RC_OK == result);
		
		/// \todo Run post-clone resolving of internal references

		return pPrefabInstance;
	}


	TDE2_API IPrefabsRegistry* CreatePrefabsRegistry(TPtr<IResourceManager> pResourceManager, TPtr<IFileSystem> pFileSystem, TPtr<IWorld> pWorld, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IPrefabsRegistry, CPrefabsRegistry, result, pResourceManager, pFileSystem, pWorld);
	}
}