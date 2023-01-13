#include "../../include/scene/CPrefabsRegistry.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/core/CEventManager.h"
#include "../../include/core/IResource.h"
#include "../../include/ecs/CWorld.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/ecs/CComponentManager.h"
#include "../../include/ecs/CEntityManager.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/scene/CPrefabsManifest.h"
#include "../../include/platform/CYAMLFile.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/editor/ecs/EditorComponents.h"
#include <algorithm> 
#include <stack>


namespace TDEngine2
{
	CPrefabsRegistry::CPrefabsRegistry() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CPrefabsRegistry::Init(IResourceManager* pResourceManager, IFileSystem* pFileSystem, IWorld* pWorld, IEventManager* pEventManager)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pResourceManager || !pFileSystem || !pWorld || !pEventManager)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = pResourceManager;
		mpFileSystem = pFileSystem;
		mpWorld = pWorld;
		mpMainEventManager = pEventManager;

		E_RESULT_CODE result = RC_OK;
		
		mpComponentsManager = TPtr<IComponentManager>(CreateComponentManager(result));
		if (RC_OK != result)
		{
			return result;
		}

		if (RC_OK != (result = mpMainEventManager->Subscribe(TOnNewComponentFactoryRegistered::GetTypeId(), this)))
		{
			return result;
		}

		mpProxyEventsManager = TPtr<IEventManager>(CreateEventManager(result));
		if (RC_OK != result)
		{
			return result;
		}

		mpEntitiesManager = TPtr<CEntityManager>(CreateEntityManager(mpProxyEventsManager.Get(), mpComponentsManager.Get(), true, result));
		if (RC_OK != result)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CPrefabsRegistry::_onFreeInternal()
	{
		mpEntitiesManager = nullptr;
		mpProxyEventsManager = nullptr; // \note The order matters because mpEntitiesManager uses raw pointer to mpProxeEventsManager

		return RC_OK;
	}


	static CPrefabsRegistry::TPrefabInfoEntity* TryGetLoadedPrefabEntity(IWorld* pWorld, CPrefabsRegistry::TPrefabsTable& loadedPrefabsTable, const std::string& id)
	{
		auto&& it = loadedPrefabsTable.find(id);
		return (it == loadedPrefabsTable.end()) ? nullptr : &it->second;
	}


	static CPrefabsRegistry::TPrefabInfoEntity LoadPrefabInfoFromManifest(IPrefabsRegistry* pPrefabsRegistry, IResourceManager* pResourceManager, IFileSystem* pFileSystem,
																			TPtr<CEntityManager>& pEntityManager, IWorld* pWorld, const std::string& id)
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
			LOG_ERROR("[CPrefabsRegistry] No loaded prefabs manifest");
			TDE2_ASSERT(false);

			return {};
		}

		/// \note If we've found one try to read archive's data
		/// \todo Make it more dependency free and type agnostic
		if (TResult<TFileEntryId> prefabFileId = pFileSystem->Open<IYAMLFileReader>(pathToPrefab))
		{
			return std::move(pPrefabsRegistry->LoadPrefabHierarchy(pFileSystem->Get<IYAMLFileReader>(prefabFileId.Get()), pEntityManager.Get()));
		}

		return {};
	}


	static CEntity* ClonePrefabHierarchy(const CPrefabsRegistry::TPrefabInfoEntity& prefabInfo, TPtr<CEntityManager>& pEntityManager, IWorld* pWorld,
										const IPrefabsRegistry::TEntityCallback& prefabEntityVisitor)
	{
		CEntity* pPrefabInstance = nullptr;

		std::unordered_map<TEntityId, TEntityId> entitiesIdsMap;

		for (auto&& currEntityId : prefabInfo.mRelatedEntities)
		{
			if (auto pOriginalEntity = pEntityManager->GetEntity(currEntityId))
			{
				if (auto pNewEntity = pWorld->CreateEntity(pOriginalEntity->GetName()))
				{
					if (prefabEntityVisitor)
					{
						prefabEntityVisitor(pNewEntity->GetId());
					}

					E_RESULT_CODE result = pOriginalEntity->Clone(pNewEntity);
					TDE2_ASSERT(RC_OK == result);

					if (currEntityId == prefabInfo.mRootEntityId)
					{
						pPrefabInstance = pNewEntity;
					}

					entitiesIdsMap.emplace(pOriginalEntity->GetId(), pNewEntity->GetId());
				}
			}
		}

		TDE2_ASSERT(pPrefabInstance);

		/// \note Run post-clone resolving of internal references
		E_RESULT_CODE result = RC_OK;

		for (TEntityId currEntityId : prefabInfo.mRelatedEntities)
		{
			if (CEntity* pEntity = pWorld->FindEntity(entitiesIdsMap[currEntityId]))
			{
				result = result | pEntity->PostLoad(pWorld->GetEntityManager(), entitiesIdsMap);
				TDE2_ASSERT(RC_OK == result);
			}
		}

		return pPrefabInstance;
	}


	CEntity* CPrefabsRegistry::Spawn(const std::string& id, CEntity* pParent, const TEntityCallback& prefabEntityVisitor)
	{
		auto pPrefabInfo = TryGetLoadedPrefabEntity(mpWorld, mPrefabsToEntityTable, id);
		if (!pPrefabInfo)
		{
			auto&& loadedPrefabInfo = LoadPrefabInfoFromManifest(this, mpResourceManager, mpFileSystem, mpEntitiesManager, mpWorld, id);
			mPrefabsToEntityTable.emplace(id, std::move(loadedPrefabInfo));

			pPrefabInfo = &mPrefabsToEntityTable[id];
		}

		if (!pPrefabInfo)
		{
			LOG_ERROR(Wrench::StringUtils::Format("[CPrefabsRegistry] The prefab {0} couldn't be loaded, there is no information about it in loaded manifests", id));
			TDE2_ASSERT(false);

			return nullptr;
		}

		CEntity* pPrefabInstance = ClonePrefabHierarchy(*pPrefabInfo, mpEntitiesManager, mpWorld, prefabEntityVisitor);

#if TDE2_EDITORS_ENABLED
		if (auto pPrefabLinkInfo = pPrefabInstance->AddComponent<CPrefabLinkInfoComponent>())
		{
			pPrefabLinkInfo->SetPrefabLinkId(id);
		}
#endif

		if (pParent)
		{
			GroupEntities(mpWorld, pParent->GetId(), pPrefabInstance->GetId());
		}

		return pPrefabInstance;
	}


#if TDE2_EDITORS_ENABLED

	static E_RESULT_CODE UpdatePrefabsRegistry(IResourceManager* pResourceManager, IFileSystem* pFileSystem, const std::string& prefabId, const std::string& prefabPath)
	{
		for (auto&& currResourceId : pResourceManager->GetResourcesListByType<CPrefabsManifest>())
		{
			if (auto pPrefabsCollection = pResourceManager->GetResource<IPrefabsManifest>(pResourceManager->GetResourceId(currResourceId)))
			{
				const std::string& pathToPrefabsManifest = dynamic_cast<IResource*>(pPrefabsCollection.Get())->GetName();
				const std::string& baseDirectoryPath = pFileSystem->GetParentPath(pathToPrefabsManifest);

				const std::string& relativePrefabPath = pFileSystem->GetRelativePath(prefabPath, pathToPrefabsManifest);

				/// \note For prefabs paths use / not \\ especially it's important for Windows
				E_RESULT_CODE result = pPrefabsCollection->AddPrefabInfo(prefabId,
					Wrench::StringUtils::ReplaceAll(
						pFileSystem->CombinePath(baseDirectoryPath, relativePrefabPath.substr(2)), 
						{ pFileSystem->GetPathSeparatorChar() }, 
						{ pFileSystem->GetAltPathSeparatorChar() })); /// remove first two symbols which're ./

				if (auto prefabsCollectionFileResult = pFileSystem->Open<IYAMLFileWriter>(pathToPrefabsManifest))
				{
					if (auto pFileWriter = pFileSystem->Get<IYAMLFileWriter>(prefabsCollectionFileResult.Get()))
					{
						result = result | pPrefabsCollection->Save(pFileWriter);
						result = result | pFileWriter->Close();
					}
				}
				else
				{
					continue; /// If file's operation failed try to write in another manifest
				}

				return result;
			}
		}

		return RC_FAIL;
	}

#endif


#if TDE2_EDITORS_ENABLED

	E_RESULT_CODE CPrefabsRegistry::SavePrefab(const std::string& id, const std::string& filePath, CEntity* pHierarchyRoot)
	{
		if (filePath.empty() || !pHierarchyRoot)
		{
			return RC_INVALID_ARGS;
		}

		E_RESULT_CODE result = RC_OK;

		TResult<TFileEntryId> prefabFileId = mpFileSystem->Open<IYAMLFileWriter>(filePath);
		if (prefabFileId.HasError())
		{
			return prefabFileId.GetError();
		}

		if (auto pFileWriter = mpFileSystem->Get<IYAMLFileWriter>(prefabFileId.Get()))
		{
			result = result | SavePrefabHierarchy(pFileWriter, mpWorld, pHierarchyRoot);
			result = result | pFileWriter->Close();
		}

		result = result | UpdatePrefabsRegistry(mpResourceManager, mpFileSystem, id, filePath); /// \note Update the manifest

		return result;
	}

	E_RESULT_CODE CPrefabsRegistry::SavePrefabHierarchy(IArchiveWriter* pWriter, IWorld* pWorld, CEntity* pEntity)
	{
		E_RESULT_CODE result = RC_OK;

		// \note Write entities
		result = result | pWriter->BeginGroup("entities", true);
		{
			std::stack<TEntityId> entitiesToProcess;

			/// \note push children's identifiers and then the identifier of the root node
			if (auto pTransform = pEntity->GetComponent<CTransform>())
			{
				for (const TEntityId childId : pTransform->GetChildren())
				{
					entitiesToProcess.push(childId);
				}

			}
			entitiesToProcess.push(pEntity->GetId());

			CTransform* pRootTransform = pEntity->GetComponent<CTransform>();

			const TEntityId parentEntityId = pRootTransform->GetParent();
			pRootTransform->SetParent(TEntityId::Invalid); /// \note Don't save relationship for the root entity

			while (!entitiesToProcess.empty())
			{
				pWriter->BeginGroup(Wrench::StringUtils::GetEmptyStr());
				{
					const TEntityId currEntityId = entitiesToProcess.top();
					entitiesToProcess.pop();

					CEntity* pCurrEntity = pWorld->FindEntity(currEntityId);
					if (!pCurrEntity)
					{
						continue;
					}

					result = result | pCurrEntity->Save(pWriter);
				}
				pWriter->EndGroup();
			}

			pRootTransform->SetParent(parentEntityId);
		}
		result = result | pWriter->EndGroup();

		return result;
	}

#endif

	CPrefabsRegistry::TPrefabInfoEntity CPrefabsRegistry::LoadPrefabHierarchy(IArchiveReader* pReader, CEntityManager* pEntityManager)
	{
		E_RESULT_CODE result = RC_OK;

		CPrefabsRegistry::TPrefabInfoEntity prefabInfo;

		std::unordered_map<TEntityId, TEntityId> entitiesIdsMap;
		auto& createdEntities = prefabInfo.mRelatedEntities;

		// \note Read entities
		result = result | pReader->BeginGroup("entities");
		{
			while (pReader->HasNextItem())
			{
				auto pNewEntity = pEntityManager->Create();
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
			if (auto pEntity = pEntityManager->GetEntity(currEntityId))
			{
				result = result | pEntity->PostLoad(pEntityManager, entitiesIdsMap);
				TDE2_ASSERT(RC_OK == result);

				if (auto pTransform = pEntity->GetComponent<CTransform>())
				{
					if (TEntityId::Invalid == pTransform->GetParent())
					{
						prefabInfo.mRootEntityId = pEntity->GetId();
					}
				}
			}
		}

		TDE2_ASSERT(RC_OK == result);
		return std::move(prefabInfo);
	}

	E_RESULT_CODE CPrefabsRegistry::OnEvent(const TBaseEvent* pEvent)
	{
		const TOnNewComponentFactoryRegistered* pConcreteEvent = dynamic_cast<const TOnNewComponentFactoryRegistered*>(pEvent);
		if (!pConcreteEvent)
		{
			TDE2_ASSERT(false);
			return RC_FAIL;
		}

		/// \note Register all user's components factories to correctly spawn the prefabs that contains them
		return mpComponentsManager->RegisterFactory(pConcreteEvent->mpFactory);
	}

	TEventListenerId CPrefabsRegistry::GetListenerId() const
	{
		return TEventListenerId(GetTypeId());
	}

	const std::vector<std::string>& CPrefabsRegistry::GetKnownPrefabsIdentifiers() const
	{
		static std::vector<std::string> prefabsList;
		prefabsList.clear();

		for (auto&& currResourceId : mpResourceManager->GetResourcesListByType<CPrefabsManifest>())
		{
			if (auto pPrefabsCollection = mpResourceManager->GetResource<IPrefabsManifest>(mpResourceManager->GetResourceId(currResourceId)))
			{
				auto&& prefabsIdentifiers = pPrefabsCollection->GetPrefabsIdentifiers();
				std::copy(prefabsIdentifiers.cbegin(), prefabsIdentifiers.cend(), std::back_inserter(prefabsList));
			}
		}

		return prefabsList;
	}


	TDE2_API IPrefabsRegistry* CreatePrefabsRegistry(IResourceManager* pResourceManager, IFileSystem* pFileSystem, IWorld* pWorld, IEventManager* pEventManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IPrefabsRegistry, CPrefabsRegistry, result, pResourceManager, pFileSystem, pWorld, pEventManager);
	}
}