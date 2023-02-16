#include "../../include/scene/CScene.h"
#include "../../include/ecs/CWorld.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/editor/CPerfProfiler.h"
#include "../../include/scene/components/CDirectionalLight.h"
#include "../../include/scene/components/CPointLight.h"
#include "../../include/scene/components/ShadowMappingComponents.h"
#include "../../include/scene/IPrefabsRegistry.h"
#include "../../include/editor/ecs/EditorComponents.h"
#include "../../include/graphics/CStaticMeshContainer.h"
#include "../../include/graphics/CStaticMesh.h"
#include "../../include/graphics/CBaseMaterial.h"
#include "../../include/graphics/CBaseCubemapTexture.h"
#include "../../include/graphics/CBaseCamera.h"
#include "../../include/graphics/CPerspectiveCamera.h"
#include "../../include/graphics/COrthoCamera.h"
#include "../../include/core/CProjectSettings.h"
#include "../../include/ecs/CTransform.h"
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <algorithm>


namespace TDEngine2
{
	struct TSceneArchiveKeys
	{
		static const std::string mPrefabLinkGroupId;

		struct TPrefabLinkGroupKeys
		{
			static const std::string mPrefabUIDKey;
			static const std::string mPrefabIdKey;
			static const std::string mParentIdKey;
			static const std::string mOverridenPositionIdKey;
		};
	};

	const std::string TSceneArchiveKeys::mPrefabLinkGroupId = "link";
	const std::string TSceneArchiveKeys::TPrefabLinkGroupKeys::mPrefabIdKey = "prefab_id";
	const std::string TSceneArchiveKeys::TPrefabLinkGroupKeys::mPrefabUIDKey = "id";
	const std::string TSceneArchiveKeys::TPrefabLinkGroupKeys::mParentIdKey = "parent_id";
	const std::string TSceneArchiveKeys::TPrefabLinkGroupKeys::mOverridenPositionIdKey = "position";


	CScene::CScene() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CScene::Init(TPtr<IWorld> pWorld, TPtr<IPrefabsRegistry> pPrefabsRegistry, const std::string& id, const std::string& scenePath, bool isMainScene)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pWorld || !pPrefabsRegistry || id.empty())
		{
			return RC_INVALID_ARGS;
		}

		mpWorld = pWorld;
		mpPrefabsRegistry = pPrefabsRegistry;

		mName = id;
		mPath = scenePath;

#if TDE2_EDITORS_ENABLED

		if (auto pSceneInfoEntity = mpWorld->CreateEntity(id + "_SceneInfoEntity"))
		{
			mSceneInfoEntityId = pSceneInfoEntity->GetId();

			if (CSceneInfoComponent* pSceneInfo = pSceneInfoEntity->AddComponent<CSceneInfoComponent>())
			{
				pSceneInfo->SetSceneId(mName);
			}
		}

#endif

		mIsMainScene = isMainScene;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CScene::_onFreeInternal()
	{
		if (!mpWorld)
		{
			return RC_FAIL;
		}

		mpWorld->Destroy(mSceneInfoEntityId);

		for (TEntityId currEntityId : mEntities)
		{
			mpWorld->Destroy(currEntityId);
		}

		return RC_OK;
	}

	E_RESULT_CODE CScene::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}

		mName = pReader->GetString("name");
		if (mAssetVersionId != pReader->GetUInt32("version"))
		{
			LOG_WARNING("[Scene] The version of the scene's asset differs from the engine's one. Loading may unexpectedly fail...");
		}

		// \note Read the properties
		pReader->BeginGroup("scene_properties");
		{
			mIsMainScene = pReader->GetBool("is_main");
		}
		pReader->EndGroup();

		E_RESULT_CODE result = CSceneLoader::LoadScene(pReader, mpWorld.Get(), this);

#if TDE2_EDITORS_ENABLED

		if (auto pSceneInfoEntity = mpWorld->FindEntity(mSceneInfoEntityId))
		{
			if (CSceneInfoComponent* pSceneInfo = pSceneInfoEntity->AddComponent<CSceneInfoComponent>())
			{
				pSceneInfo->SetSceneId(mName);
			}
		}

#endif

		return result;
	}

	E_RESULT_CODE CScene::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		pWriter->SetString("name", mName);
		pWriter->SetUInt32("version", mAssetVersionId); // unused for now
		
		// \note Write down the properties
		pWriter->BeginGroup("scene_properties");
		{
			pWriter->SetBool("is_main", mIsMainScene);
		}
		pWriter->EndGroup();

		// \note Write down entities
		CSceneSerializer::SaveScene(pWriter, mpWorld, this);

		return RC_OK;
	}

	CEntity* CScene::CreateEntity(const std::string& name)
	{
		if (!mpWorld)
		{
			return nullptr;
		}

		CEntity* pEntity = mpWorld->CreateEntity(name);
		TDE2_ASSERT(pEntity);

		mEntities.push_back(pEntity->GetId());

		return pEntity;
	}

	E_RESULT_CODE CScene::RemoveEntity(TEntityId id)
	{
		if (id == TEntityId::Invalid)
		{
			return RC_INVALID_ARGS;
		}

		const bool entityBelongsToScene = std::find(mEntities.cbegin(), mEntities.cend(), id) != mEntities.cend();

		if (!mpWorld || !entityBelongsToScene)
		{
			if (!entityBelongsToScene)
			{
				LOG_ERROR(Wrench::StringUtils::Format("[Scene] RemoveEntity(id), The given entity doesn't belong to current scene; id : {0}; scene: {1}", static_cast<U32>(id), mName));
			}

			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		if (RC_OK != (result = mpWorld->Destroy(id)))
		{
			return result;
		}

		/// \note Destroy can remove all the child entities so we should check all entities of the scene
		mEntities.erase(std::remove_if(mEntities.begin(), mEntities.end(), [this, entityIdToRemove = id](const TEntityId& entityId)
		{
			return entityIdToRemove == entityId || !mpWorld->FindEntity(entityId);
		}), mEntities.cend());

		return RC_OK;
	}

	CEntity* CScene::CreateDirectionalLight(const TColor32F& tint, F32 intensity, const TVector3& direction)
	{
		CEntity* pDirectionLightEntity = CreateEntity("DirectionLight");

		if (auto pDirectionLightComponent = pDirectionLightEntity->AddComponent<CDirectionalLight>())
		{
			pDirectionLightComponent->SetColor(tint);
			pDirectionLightComponent->SetIntensity(intensity);
		}

		return pDirectionLightEntity;
	}

	CEntity* CScene::CreatePointLight(const TColor32F& tint, F32 intensity, F32 range)
	{
		CEntity* pPointLightEntity = CreateEntity("PointLight");

		if (auto pPointLight = pPointLightEntity->AddComponent<CPointLight>())
		{
			pPointLight->SetColor(tint);
			pPointLight->SetIntensity(intensity);
			pPointLight->SetRange(range);
		}

		return pPointLightEntity;
	}

	CEntity* CScene::CreateSkybox(IResourceManager* pResourceManager, const std::string& skyboxTexture)
	{
		if (!pResourceManager)
		{
			return nullptr;
		}

		auto skyboxEntities = mpWorld->FindEntitiesWithComponents<CSkyboxComponent>();
		if (!skyboxEntities.empty())
		{
			return mpWorld->FindEntity(skyboxEntities.front());
		}

		CEntity* pSkyboxEntity = CreateEntity("Skybox");

		pSkyboxEntity->AddComponent<CSkyboxComponent>();

		if (auto pStaticMeshContainer = pSkyboxEntity->AddComponent<CStaticMeshContainer>())
		{
			pStaticMeshContainer->SetMaterialName(CProjectSettings::Get()->mGraphicsSettings.mDefaultSkyboxMaterial);
			pStaticMeshContainer->SetMeshName("Cube");
		}

		return pSkyboxEntity;
	}

#if TDE2_EDITORS_ENABLED

	CEntity* CScene::CreateEditorCamera(F32 aspect, F32 fov)
	{
		CEntity* pEditorCameraEntity = mpWorld->CreateEntity("EditorCamera");

		if (auto pCamera = pEditorCameraEntity->AddComponent<CPerspectiveCamera>())
		{
			pCamera->SetAspect(aspect);
			pCamera->SetFOV(fov);
		}		

		pEditorCameraEntity->AddComponent<CEditorCamera>();

		return pEditorCameraEntity;
	}

#endif

	CEntity* CScene::CreateCamera(const std::string& id, E_CAMERA_PROJECTION_TYPE cameraType, const TBaseCameraParameters& params)
	{
		CEntity* pCameraEntity = CreateEntity(id);

		switch (cameraType)
		{
			case E_CAMERA_PROJECTION_TYPE::PERSPECTIVE:
				if (auto pCamera = pCameraEntity->AddComponent<CPerspectiveCamera>())
				{
					auto&& perspectiveCameraParams = dynamic_cast<const TPerspectiveCameraParameters&>(params);

					pCamera->SetAspect(perspectiveCameraParams.mAspect);
					pCamera->SetFOV(perspectiveCameraParams.mFOV);
					pCamera->SetNearPlane(perspectiveCameraParams.mZNear);
					pCamera->SetFarPlane(perspectiveCameraParams.mZFar);
				}

				return pCameraEntity;

			case E_CAMERA_PROJECTION_TYPE::ORTHOGRAPHIC:
				if (auto pCamera = pCameraEntity->AddComponent<COrthoCamera>())
				{
					auto&& orthoCameraParams = dynamic_cast<const TOrthoCameraParameters&>(params);

					pCamera->SetWidth(orthoCameraParams.mViewportWidth);
					pCamera->SetHeight(orthoCameraParams.mViewportHeight);
					pCamera->SetNearPlane(orthoCameraParams.mZNear);
					pCamera->SetFarPlane(orthoCameraParams.mZFar);
				}

				return pCameraEntity;
		}

		TDE2_UNREACHABLE();
		return nullptr;
	}

	CEntity* CScene::Spawn(const std::string& prefabId, CEntity* pParentEntity)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		return mpPrefabsRegistry->Spawn(prefabId, pParentEntity, [this](const TEntityId& id)
		{
			mEntities.push_back(id);
		});
	}

	void CScene::ForEachEntity(const std::function<void(CEntity*)>& action)
	{
		TDE2_PROFILER_SCOPE("CScene::ForEachEntity");
		std::lock_guard<std::mutex> lock(mMutex);

		if (!action)
		{
			return; // Do nothing
		}

		for (TEntityId currEntityId : mEntities)
		{
			CEntity* pCurrEntity = mpWorld->FindEntity(currEntityId);
			if (!pCurrEntity)
			{
				continue;
			}

			action(pCurrEntity);
		}
	}

	const std::string& CScene::GetName() const
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return mName;
	}

	const std::string& CScene::GetScenePath() const
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return mPath;
	}

	bool CScene::IsMainScene() const
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return mIsMainScene;
	}

	const std::vector<TEntityId>& CScene::GetEntities() const
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return mEntities;
	}


	TDE2_API IScene* CreateScene(TPtr<IWorld> pWorld, TPtr<IPrefabsRegistry> pPrefabsRegistry, const std::string& id, const std::string& scenePath, bool isMainScene, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IScene, CScene, result, pWorld, pPrefabsRegistry, id, scenePath, isMainScene);
	}

	
	/*!
		\brief The CSceneLoader's implementation
	*/


	struct TLoadEntitiesInfo
	{
		std::vector<TEntityId>                                            mCreatedEntities;
		std::vector<TEntityId>                                            mRootEntities;
		std::vector<IPrefabsRegistry::TPrefabInfoEntity::TPrefabLinkInfo> mPrefabsLinks;
	};


	static TResult<TLoadEntitiesInfo> LoadEntitiesImpl(
		IArchiveReader* pReader, 
		CEntityManager* pEntityManager, 
		const std::function<CEntity*()>& entityFactory, 
		const std::function<CEntity*(const std::string&, CEntity*)>& prefabLinkFactory)
	{
		TDE2_ASSERT(pReader);
		TDE2_ASSERT(pEntityManager);
		TDE2_ASSERT(entityFactory);
		TDE2_ASSERT(prefabLinkFactory);

		TLoadEntitiesInfo output;

		E_RESULT_CODE result = RC_OK;

		TEntitiesMapper entitiesIdsMap;
		std::unordered_set<TEntityId> nonPrefabEntitiesSet;

		std::queue<IPrefabsRegistry::TPrefabInfoEntity::TPrefabLinkInfo> prefabsDefferedSpawnQueue; /// all prefabs are spawned after all the scene has been read

		auto& createdEntities = output.mCreatedEntities;

		// \note Read entities
		pReader->BeginGroup("entities");
		{
			while (pReader->HasNextItem())
			{
				pReader->BeginGroup(Wrench::StringUtils::GetEmptyStr());
				{
					TEntityId entityId = TEntityId::Invalid;

					pReader->BeginGroup("entity"); /// \note Read id first, it's a tag which data block we should read next
					{
						entityId = static_cast<TEntityId>(pReader->GetUInt32("id", static_cast<U32>(TEntityId::Invalid)));
					}
					pReader->EndGroup();

					if (TEntityId::Invalid == entityId) /// \note If the entityId is empty then try to read prefab
					{
						pReader->BeginGroup(TSceneArchiveKeys::mPrefabLinkGroupId);
						{
							pReader->BeginGroup(TSceneArchiveKeys::TPrefabLinkGroupKeys::mOverridenPositionIdKey);
							auto positionResult = LoadVector3(pReader); /// \note Try to read overriden position for the link
							pReader->EndGroup();

							TEntityId originalParentEntityId = static_cast<TEntityId>(
								pReader->GetUInt32(TSceneArchiveKeys::TPrefabLinkGroupKeys::mParentIdKey, static_cast<U32>(TEntityId::Invalid)));

							TVector3 overridenPosition = ZeroVector3;

							if (positionResult.IsOk())
							{
								overridenPosition = positionResult.Get();
							}

							prefabsDefferedSpawnQueue.push(
								{
									overridenPosition,
									static_cast<TEntityId>(pReader->GetUInt32("id", static_cast<U32>(TEntityId::Invalid))),
									originalParentEntityId,
									pReader->GetString(TSceneArchiveKeys::TPrefabLinkGroupKeys::mPrefabIdKey)
								});
						}
						pReader->EndGroup();
					}
					else
					{
						CEntity* pNewEntity = entityFactory();
						createdEntities.emplace_back(pNewEntity->GetId());

						pReader->BeginGroup("entity");
						{
							// \todo Implement remapping of entity's identifiers
							entitiesIdsMap.mSerializedToRuntimeIdsTable.emplace(entityId, pNewEntity->GetId());
							nonPrefabEntitiesSet.emplace(pNewEntity->GetId());
						}
						pReader->EndGroup();

						pNewEntity->Load(pReader);
					}
				}
				pReader->EndGroup();
			}
		}
		pReader->EndGroup();

		// \note Post load remapping stage
		auto& rootEntities = output.mRootEntities;

		for (TEntityId currEntityId : createdEntities)
		{
			if (nonPrefabEntitiesSet.find(currEntityId) == nonPrefabEntitiesSet.cend())
			{
				rootEntities.push_back(currEntityId);
				continue; // \note If there is no information about the entity's identifier that means it's a prefab. It's been already resolved so there is no need for post-process step
			}

			if (auto pEntity = pEntityManager->GetEntity(currEntityId))
			{
				auto pTransform = pEntity->GetComponent<CTransform>();
				TDE2_ASSERT(pTransform);

				if (!pTransform)
				{
					continue;
				}

#if TDE2_EDITORS_ENABLED
				/// \note Remove all child which are prefabs links. They'll be attached later after spawning
				std::vector<TEntityId> childrenToExclude;
					
				for (const TEntityId& currChildId : pTransform->GetChildren())
				{
					if (entitiesIdsMap.mSerializedToRuntimeIdsTable.find(currChildId) != entitiesIdsMap.mSerializedToRuntimeIdsTable.end())
					{
						continue;
					}

					childrenToExclude.push_back(currChildId);
				}

				for (const TEntityId& currPrefabLinkId : childrenToExclude)
				{
					pTransform->DettachChild(currPrefabLinkId);
				}
#endif

				result = result | pEntity->PostLoad(pEntityManager, entitiesIdsMap);

				TDE2_ASSERT(RC_OK == result);

				if (TEntityId::Invalid == pTransform->GetParent())
				{
					rootEntities.push_back(pEntity->GetId());
				}
			}
		}

		/// \note Spawn prefabs
		while (!prefabsDefferedSpawnQueue.empty())
		{
			auto& currPrefabInfo = prefabsDefferedSpawnQueue.front();

			if (TEntityId::Invalid != currPrefabInfo.mParentId)
			{
				currPrefabInfo.mParentId = entitiesIdsMap.Resolve(currPrefabInfo.mParentId);  /// \note Convert the serialized value to the runtime one
			}

			output.mPrefabsLinks.push_back(currPrefabInfo);

			auto pInstance = prefabLinkFactory(currPrefabInfo.mPrefabId, pEntityManager->GetEntity(currPrefabInfo.mParentId).Get());
			if (!pInstance)
			{
				prefabsDefferedSpawnQueue.pop();
				continue;
			}

			if (TEntityId::Invalid == currPrefabInfo.mParentId)
			{
				rootEntities.push_back(pInstance->GetId());
			}

			if (Length(currPrefabInfo.mPosition) > FloatEpsilon)
			{
				CTransform* pTransform = pInstance->GetComponent<CTransform>();
				pTransform->SetPosition(currPrefabInfo.mPosition);
			}

			prefabsDefferedSpawnQueue.pop();

			entitiesIdsMap.mSerializedToRuntimeIdsTable.emplace(currPrefabInfo.mId, pInstance->GetId());
		}

		if (RC_OK != result)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		return Wrench::TOkValue<TLoadEntitiesInfo>(output);
	}


	E_RESULT_CODE CSceneLoader::LoadScene(IArchiveReader* pReader, IWorld* pWorld, IScene* pScene)
	{
		auto&& loadResult = LoadEntitiesImpl(
			pReader,
			pWorld->GetEntityManager(),
			[pScene]
			{
				return pScene->CreateEntity(Wrench::StringUtils::GetEmptyStr());
			},
			[pScene](const std::string& prefabId, CEntity* pParentEntity)
			{
				return pScene->Spawn(prefabId, pParentEntity);
			});

		return loadResult.IsOk() ? RC_OK : loadResult.GetError();
	}

	TResult<IPrefabsRegistry::TPrefabInfoEntity> CSceneLoader::LoadPrefab(
		IArchiveReader* pReader,
		CEntityManager* pEntityManager,
		const IPrefabsRegistry::TEntityFactoryFunctor& entityFactory,
		const IPrefabsRegistry::TPrefabFactoryFunctor& prefabFactory)
	{
		TDE2_ASSERT(pReader);
		TDE2_ASSERT(pEntityManager);
		TDE2_ASSERT(entityFactory);
		TDE2_ASSERT(prefabFactory);

		IPrefabsRegistry::TPrefabInfoEntity output;

		auto&& loadResult = LoadEntitiesImpl(pReader, pEntityManager, entityFactory, prefabFactory);
		if (loadResult.HasError())
		{
			return Wrench::TErrValue<E_RESULT_CODE>(loadResult.GetError());
		}

		auto&& loadInfo = loadResult.Get();
		TDE2_ASSERT(!loadInfo.mRootEntities.empty());

		if (loadInfo.mRootEntities.empty())
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		output.mRelatedEntities = std::move(loadInfo.mCreatedEntities);
		output.mRootEntityId = loadInfo.mRootEntities.front();
		output.mNestedPrefabsLinks = std::move(loadInfo.mPrefabsLinks);

		return Wrench::TOkValue<IPrefabsRegistry::TPrefabInfoEntity>(output);
	}


	/*!
		\brief CSceneSerializer's definition
	*/


	static E_RESULT_CODE SaveSingleEntityImpl(IArchiveWriter* pWriter, TPtr<IWorld> pWorld, TEntityId currEntityId, bool savePrefabAsSimpleEntity = false)
	{
		E_RESULT_CODE result = RC_OK;

		CEntity* pCurrEntity = pWorld->FindEntity(currEntityId);
		if (!pCurrEntity)
		{
			return RC_INVALID_ARGS;
		}

		result = result | pWriter->BeginGroup(Wrench::StringUtils::GetEmptyStr());
		{
#if TDE2_EDITORS_ENABLED
			auto pPrefabLinkInfo = pCurrEntity->GetComponent<CPrefabLinkInfoComponent>();
			if (!savePrefabAsSimpleEntity && pPrefabLinkInfo)
			{
				result = result | pWriter->BeginGroup(TSceneArchiveKeys::mPrefabLinkGroupId);
				{
					result = result | pWriter->SetString(TSceneArchiveKeys::TPrefabLinkGroupKeys::mPrefabIdKey, pPrefabLinkInfo->GetPrefabLinkId());
					result = result | pWriter->SetUInt32(TSceneArchiveKeys::TPrefabLinkGroupKeys::mPrefabUIDKey, static_cast<U32>(currEntityId));

					if (auto pTransform = pCurrEntity->GetComponent<CTransform>())
					{
						if (TEntityId::Invalid != pTransform->GetParent())
						{
							pWriter->SetUInt32(TSceneArchiveKeys::TPrefabLinkGroupKeys::mParentIdKey, static_cast<U32>(pTransform->GetParent()));
						}

						result = result | pWriter->BeginGroup(TSceneArchiveKeys::TPrefabLinkGroupKeys::mOverridenPositionIdKey);
						result = result | SaveVector3(pWriter, pTransform->GetPosition());
						result = result | pWriter->EndGroup();
					}
				}
				result = result | pWriter->EndGroup();
			}
			else
#endif
			{
				result = result | pCurrEntity->Save(pWriter);
			}
		}
		result = result | pWriter->EndGroup();

		return RC_OK;
	}


	E_RESULT_CODE CSceneSerializer::SaveScene(IArchiveWriter* pWriter, TPtr<IWorld> pWorld, IScene* pScene)
	{
		E_RESULT_CODE result = RC_OK;

		if (!pWriter || !pScene || !pWorld)
		{
			return RC_INVALID_ARGS;
		}

		result = result | pWriter->BeginGroup("entities", true);
		{
			for (TEntityId currEntityId : pScene->GetEntities())
			{
				const TEntityId prefabEntityRootId =
#if TDE2_EDITORS_ENABLED
					GetPrefabInstanceRootEntityId(pWorld, currEntityId);
#else
					TEntityId::Invalid;
#endif
				if (TEntityId::Invalid != prefabEntityRootId && prefabEntityRootId != currEntityId) /// \note If it's a part of a prefab but not it's root skip serialization process
				{
					continue;
				}

				result = result | SaveSingleEntityImpl(pWriter, pWorld, currEntityId);
			}
		}
		result = result | pWriter->EndGroup();

		return result;
	}

	E_RESULT_CODE CSceneSerializer::SavePrefab(IArchiveWriter* pWriter, TPtr<IWorld> pWorld, CEntity* pRootEntity)
	{
		E_RESULT_CODE result = RC_OK;

		// \note Write entities
		result = result | pWriter->BeginGroup("entities", true);
		{
			std::stack<TEntityId> entitiesToProcess;

			/// \note push children's identifiers and then the identifier of the root node
			if (auto pTransform = pRootEntity->GetComponent<CTransform>())
			{
				for (const TEntityId childId : pTransform->GetChildren())
				{
					entitiesToProcess.push(childId);
				}

			}
			entitiesToProcess.push(pRootEntity->GetId());

			CTransform* pRootTransform = pRootEntity->GetComponent<CTransform>();

			const TEntityId parentEntityId = pRootTransform->GetParent();
			pRootTransform->SetParent(TEntityId::Invalid); /// \note Don't save relationship for the root entity

			while (!entitiesToProcess.empty())
			{
				const TEntityId currEntityId = entitiesToProcess.top();
				entitiesToProcess.pop();

				const TEntityId prefabEntityRootId =
#if TDE2_EDITORS_ENABLED
					GetPrefabInstanceRootEntityId(pWorld, currEntityId);
#else
					TEntityId::Invalid;
#endif

				if (TEntityId::Invalid != prefabEntityRootId && 
					prefabEntityRootId != currEntityId && 
					prefabEntityRootId != pRootEntity->GetId()) /// \note If it's a part of a prefab but not it's root skip serialization process
				{
					continue;
				}

				result = result | SaveSingleEntityImpl(pWriter, pWorld, currEntityId, currEntityId == pRootEntity->GetId());

				if (CEntity* pCurrEntity = pWorld->FindEntity(currEntityId))
				{
					if (auto pTransform = pCurrEntity->GetComponent<CTransform>())
					{
						if (pRootTransform == pTransform)
						{
							continue;
						}

						for (const TEntityId childId : pTransform->GetChildren())
						{
							entitiesToProcess.push(childId);
						}
					}
				}				
			}

			pRootTransform->SetParent(parentEntityId);
		}
		result = result | pWriter->EndGroup();

		return result;
	}
}