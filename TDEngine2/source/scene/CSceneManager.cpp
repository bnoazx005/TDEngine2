#include "../../include/scene/CSceneManager.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/scene/CScene.h"
#include "../../include/core/IFile.h"
#include "../../include/core/IJobManager.h"
#include "../../include/editor/CPerfProfiler.h"
#include <algorithm>


namespace TDEngine2
{
	CSceneManager::CSceneManager() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CSceneManager::Init(TPtr<IFileSystem> pFileSystem, TPtr<IWorld> pWorld, TPtr<IPrefabsRegistry> pPrefabsRegistry, const TSceneManagerSettings& settings)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pFileSystem || !pWorld)
		{
			return RC_INVALID_ARGS;
		}

		mSettings = settings;

		mpFileSystem = pFileSystem;
		mpWorld = pWorld;
		mpPrefabsRegistry = pPrefabsRegistry;

		mIsInitialized = true;

		return _onPostInit();
	}

	E_RESULT_CODE CSceneManager::_onFreeInternal()
	{
		E_RESULT_CODE result = RC_OK;

		for (IScene* pCurrScene : mpScenes)
		{
			if (!pCurrScene)
			{
				continue;
			}

			result = result | pCurrScene->Free();
		}

		mpScenes.clear();
		mpScenesHandlesTable.clear();

		return result;
	}

	TResult<TSceneId> CSceneManager::LoadScene(const std::string& scenePath)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		return _loadSceneInternal(scenePath);
	}

	TResult<TSceneId> CSceneManager::_loadSceneInternal(const std::string& scenePath)
	{
		const std::string& sceneName = mpFileSystem->ExtractFilename(scenePath);

		// \note If there is loaded scene then just return its handle
		auto iter = mpScenesHandlesTable.find(sceneName);
		if (iter != mpScenesHandlesTable.cend())
		{
			return Wrench::TOkValue<TSceneId>(iter->second);
		}

		E_RESULT_CODE result = RC_OK;

		IScene* pScene = TDEngine2::CreateScene(mpWorld, mpPrefabsRegistry, sceneName, scenePath, mpScenes.empty(), result); // \todo Add check up for a main scene flag

		if (RC_OK != result || !pScene)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		// \note Open scene's file and read its data
		if (auto openSceneFileResult = mpFileSystem->Open<IYAMLFileReader>(scenePath))
		{
			if (IYAMLFileReader* pSceneReader = mpFileSystem->Get<IYAMLFileReader>(openSceneFileResult.Get()))
			{
				result = pScene->Load(pSceneReader);
				pSceneReader->Close();
			}
			else
			{
				result = RC_FAIL;
				TDE2_ASSERT(false);
			}
		}

		if (RC_OK != result)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		return _registerSceneInternal(sceneName, pScene);
	}

	void CSceneManager::LoadSceneAsync(const std::string& scenePath, const TLoadSceneCallback& onResultCallback)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		IJobManager* pJobManager = mpFileSystem->GetJobManager();
		if (!pJobManager)
		{
			if (onResultCallback)
			{
				onResultCallback(Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS));
			}

			return;
		}

		pJobManager->SubmitJob(nullptr, [this, scenePath, onResultCallback, pJobManager](auto&&)
		{
			TDE2_PROFILER_SCOPE("LoadSceneAsyncJob");

			const std::string& sceneName = mpFileSystem->ExtractFilename(scenePath);

			// \note If there is loaded scene then just return its handle
			auto iter = std::find_if(mpScenes.cbegin(), mpScenes.cend(), [&scenePath](const IScene* pScene) { return pScene && pScene->GetScenePath() == scenePath; });
			if (onResultCallback && iter != mpScenes.cend())
			{
				/// \note This callback should be executed in the main thread
				pJobManager->ExecuteInMainThread([onResultCallback, sceneId = static_cast<TSceneId>(std::distance(mpScenes.cbegin(), iter))] 
					{
						onResultCallback(Wrench::TOkValue<TSceneId>(sceneId));
					});

				return;
			}

			auto openSceneFileResult = mpFileSystem->Open<IYAMLFileReader>(scenePath);
			if (openSceneFileResult.HasError()) // \note There is no a scene with such identifier
			{
				if (onResultCallback)
				{
					onResultCallback(Wrench::TErrValue<E_RESULT_CODE>(openSceneFileResult.GetError()));
				}

				return;
			}

			E_RESULT_CODE result = RC_OK;

			IScene* pScene = TDEngine2::CreateScene(mpWorld, mpPrefabsRegistry, sceneName, scenePath, mpScenes.empty(), result); // \todo Add check up for a main scene flag

			if (onResultCallback && (RC_OK != result || !pScene))
			{
				/// \note This callback should be executed in the main thread
				pJobManager->ExecuteInMainThread([onResultCallback, result] { onResultCallback(Wrench::TErrValue<E_RESULT_CODE>(result)); });

				return;
			}

			// \note Open scene's file and read its data
			if (openSceneFileResult.IsOk())
			{
				if (IYAMLFileReader* pSceneReader = mpFileSystem->Get<IYAMLFileReader>(openSceneFileResult.Get()))
				{
					result = pScene->Load(pSceneReader);
					pSceneReader->Close();
				}
				else
				{
					result = RC_FAIL;
					TDE2_ASSERT(false);
				}
			}

			if (onResultCallback && RC_OK != result)
			{
				/// \note This callback should be executed in the main thread
				pJobManager->ExecuteInMainThread([onResultCallback, result] { onResultCallback(Wrench::TErrValue<E_RESULT_CODE>(result)); });

				return;
			}

			/// \note This callback should be executed in the main thread
			pJobManager->ExecuteInMainThread([onResultCallback, this, sceneName, pScene] 
			{ 
				auto result = _registerSceneInternal(sceneName, pScene);

				if (onResultCallback)
				{
					onResultCallback(result);
				}
			});
		}, { E_JOB_PRIORITY_TYPE::NORMAL, true });
	}

	E_RESULT_CODE CSceneManager::UnloadScene(TSceneId id)
	{
		if (auto findSceneResult = GetScene(id))
		{
			if (IScene* pScene = findSceneResult.Get())
			{
				E_RESULT_CODE result = _unregisterSceneInternal(id);
				result = result | pScene->Free();

				return result;
			}

			TDE2_ASSERT(false);
		}

		return RC_FAIL;
	}

	TResult<TSceneId> CSceneManager::CreateScene(const std::string& name)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _createInternal(name);
	}

#if TDE2_EDITORS_ENABLED
	
	E_RESULT_CODE CSceneManager::SaveSceneChunk(const std::string& path, TSceneId sceneId)
	{
		auto sceneResult = GetScene(sceneId);
		if (sceneResult.HasError())
		{
			return sceneResult.GetError();
		}

		auto pScene = sceneResult.Get();

		if (auto result = mpFileSystem->Open<IYAMLFileWriter>(path, true))
		{
			if (auto pFileWriter = mpFileSystem->Get<IYAMLFileWriter>(result.Get()))
			{
				pScene->Save(pFileWriter);
				pFileWriter->Close();
			}
		}

		return RC_OK;
	}

#endif

	TResult<IScene*> CSceneManager::GetScene(TSceneId id) const
	{
		std::lock_guard<std::mutex> lock(mMutex);

		const U32 index = static_cast<U32>(id);

		if ((id == TSceneId::Invalid) || (index >= mpScenes.size()))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		return Wrench::TOkValue<IScene*>(mpScenes[index]);
	}

	TSceneId CSceneManager::GetSceneId(const std::string& id) const
	{
		std::lock_guard<std::mutex> lock(mMutex);

		auto it = mpScenesHandlesTable.find(id);
		return (it == mpScenesHandlesTable.cend()) ? TSceneId::Invalid : it->second;
	}

	TPtr<IWorld> CSceneManager::GetWorld() const
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return mpWorld;
	}

	TPtr<IPrefabsRegistry> CSceneManager::GetPrefabsRegistry() const
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return mpPrefabsRegistry;
	}

	const CSceneManager::TScenesArray& CSceneManager::GetLoadedScenes() const
	{
		std::lock_guard<std::mutex> lock(mMutex);

		return mpScenes;
	}

	TResult<TSceneId> CSceneManager::_createInternal(const std::string& name)
	{
		E_RESULT_CODE result = RC_OK;

		IScene* pScene = TDEngine2::CreateScene(mpWorld, mpPrefabsRegistry, name, Wrench::StringUtils::GetEmptyStr(), mpScenes.empty(), result);

		if (RC_OK != result)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		return _registerSceneInternal(name, pScene);
	}

	TResult<TSceneId> CSceneManager::_registerSceneInternal(const std::string& name, IScene* pScene)
	{
		auto iter = mpScenesHandlesTable.find(name);
		if (iter != mpScenesHandlesTable.cend())
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		/// \note seek for the first free place
		auto it = std::find(mpScenes.begin(), mpScenes.end(), nullptr);
		
		const TSceneId sceneHandle = TSceneId(std::distance(mpScenes.begin(), it));

		if (it == mpScenes.end())
		{
			mpScenes.push_back(pScene);
		}
		else
		{
			*it = pScene;
		}
		
		mpScenesHandlesTable.emplace(pScene->GetName(), sceneHandle);

		return Wrench::TOkValue<TSceneId>(sceneHandle);
	}

	E_RESULT_CODE CSceneManager::_unregisterSceneInternal(TSceneId id)
	{
		const USIZE index = static_cast<USIZE>(id);

		if (TSceneId::Invalid == id || index >= mpScenes.size())
		{
			return RC_INVALID_ARGS;
		}

		IScene* pSceneToRemove = mpScenes[index];
		if (!pSceneToRemove)
		{
			return RC_FAIL;
		}

		mpScenes[index] = nullptr;
		mpScenesHandlesTable.erase(pSceneToRemove->GetName());

		return RC_OK;
	}

	TDE2_API E_RESULT_CODE CSceneManager::_onPostInit()
	{
		auto createEmptyMainScene = [this](const std::string& scenePath) 
		{
			const std::string& sceneName = mpFileSystem->ExtractFilename(scenePath);

			// \create a new empty scene		
			auto mainSceneCreationResult = _createInternal(sceneName);
			if (mainSceneCreationResult.HasError())
			{
				return mainSceneCreationResult.GetError();
			}

			return RC_OK;
		};

		// \note Load main scene in synchronous fashion
		if (auto openSceneFileResult = mpFileSystem->Open<IYAMLFileReader>(mSettings.mMainScenePath)) // \note File exists, so we load it
		{
			auto mainSceneLoadingResult = _loadSceneInternal(mSettings.mMainScenePath);
			if (mainSceneLoadingResult.HasError())
			{				
				return createEmptyMainScene(mSettings.mMainScenePath); /// \note Create a default empty scene and mark that as persistent
			}

			return RC_OK;
		}

		return createEmptyMainScene(mSettings.mMainScenePath);
	}


	TDE2_API ISceneManager* CreateSceneManager(TPtr<IFileSystem> pFileSystem, TPtr<IWorld> pWorld, TPtr<IPrefabsRegistry> pPrefabsRegistry, const TSceneManagerSettings& settings, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISceneManager, CSceneManager, result, pFileSystem, pWorld, pPrefabsRegistry, settings);
	}
}