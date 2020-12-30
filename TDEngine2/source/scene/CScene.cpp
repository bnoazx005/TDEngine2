#include "../../include/scene/CScene.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/scene/components/CDirectionalLight.h"
#include "../../include/editor/ecs/EditorComponents.h"
#include <unordered_map>


namespace TDEngine2
{
	CScene::CScene() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CScene::Init(IWorld* pWorld, const std::string& id, const std::string& scenePath, bool isMainScene)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pWorld || id.empty())
		{
			return RC_INVALID_ARGS;
		}

		mpWorld = pWorld;

		mName = id;
		mPath = scenePath;

		if (auto pSceneInfoEntity = mpWorld->CreateEntity(id + "_SceneInfoEntity"))
		{
			mSceneInfoEntityId = pSceneInfoEntity->GetId();

			if (CSceneInfoComponent* pSceneInfo = pSceneInfoEntity->AddComponent<CSceneInfoComponent>())
			{
				pSceneInfo->SetSceneId(mName);
			}
		}

		mIsMainScene = isMainScene;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CScene::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		--mRefCounter;

		if (!mRefCounter)
		{
			if (!mpWorld)
			{
				return RC_FAIL;
			}

			mpWorld->Destroy(mpWorld->FindEntity(mSceneInfoEntityId));

			for (TEntityId currEntityId : mEntities)
			{
				mpWorld->Destroy(mpWorld->FindEntity(currEntityId));
			}

			mIsInitialized = false;
			delete this;
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

		// \note Write down the properties
		pReader->BeginGroup("scene_properties");
		{
			mIsMainScene = pReader->GetBool("is_main");
		}
		pReader->EndGroup();

		std::unordered_map<TEntityId, TEntityId> entitiesIdsMap;

		// \note Write down entities
		pReader->BeginGroup("entities");
		{
			while (pReader->HasNextItem())
			{
				CEntity* pNewEntity = mpWorld->CreateEntity();

				pReader->BeginGroup(Wrench::StringUtils::GetEmptyStr());
				{
					pReader->BeginGroup("entity");
					{
						// \todo Implement remapping of entity's identifiers
						entitiesIdsMap.emplace(static_cast<TEntityId>(pReader->GetUInt32("id")), pNewEntity->GetId());
					}
					pReader->EndGroup();

					pNewEntity->Load(pReader);
				}
				pReader->EndGroup();

				mEntities.push_back(pNewEntity->GetId());
			}
		}
		pReader->EndGroup();

		if (auto pSceneInfoEntity = mpWorld->FindEntity(mSceneInfoEntityId))
		{
			if (CSceneInfoComponent* pSceneInfo = pSceneInfoEntity->AddComponent<CSceneInfoComponent>())
			{
				pSceneInfo->SetSceneId(mName);
			}
		}

		return RC_OK;
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
		pWriter->BeginGroup("entities", true);
		{
			for (TEntityId currEntityId : mEntities)
			{
				if (CEntity* pCurrEntity = mpWorld->FindEntity(currEntityId))
				{
					pWriter->BeginGroup(Wrench::StringUtils::GetEmptyStr());
					{
						pCurrEntity->Save(pWriter);
					}
					pWriter->EndGroup();
				}
			}
		}
		pWriter->EndGroup();

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

		if (RC_OK != (result = mpWorld->Destroy(mpWorld->FindEntity(id))))
		{
			return result;
		}

		mEntities.erase(std::find(mEntities.cbegin(), mEntities.cend(), id));

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

	void CScene::ForEachEntity(const std::function<void(CEntity*)>& action)
	{
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


	TDE2_API IScene* CreateScene(IWorld* pWorld, const std::string& id, const std::string& scenePath, bool isMainScene, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IScene, CScene, result, pWorld, id, scenePath, isMainScene);
	}
}