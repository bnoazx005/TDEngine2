#include "../../include/scene/CScene.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/editor/CPerfProfiler.h"
#include "../../include/scene/components/CDirectionalLight.h"
#include "../../include/scene/components/CPointLight.h"
#include "../../include/scene/components/ShadowMappingComponents.h"
#include "../../include/editor/ecs/EditorComponents.h"
#include "../../include/graphics/CStaticMeshContainer.h"
#include "../../include/graphics/CStaticMesh.h"
#include "../../include/graphics/CBaseMaterial.h"
#include "../../include/graphics/CBaseCubemapTexture.h"
#include "../../include/graphics/CBaseCamera.h"
#include "../../include/graphics/CPerspectiveCamera.h"
#include "../../include/graphics/COrthoCamera.h"
#include "../../include/core/CProjectSettings.h"
#include <unordered_map>


namespace TDEngine2
{
	CScene::CScene() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CScene::Init(TPtr<IWorld> pWorld, const std::string& id, const std::string& scenePath, bool isMainScene)
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

	E_RESULT_CODE CScene::_onFreeInternal()
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

		// \note Post load remapping stage
		E_RESULT_CODE result = RC_OK;

		for (TEntityId currEntityId : mEntities)
		{
			if (CEntity* pEntity = mpWorld->FindEntity(currEntityId))
			{
				result = result | pEntity->PostLoad(mpWorld.Get(), entitiesIdsMap);

				TDE2_ASSERT(RC_OK == result);
			}
		}

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


	TDE2_API IScene* CreateScene(TPtr<IWorld> pWorld, const std::string& id, const std::string& scenePath, bool isMainScene, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IScene, CScene, result, pWorld, id, scenePath, isMainScene);
	}
}