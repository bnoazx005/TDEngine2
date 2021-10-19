#include "../../include/scene/CSceneManager.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/scene/CScene.h"
#include "../../include/core/IFile.h"
#include "../../include/core/IJobManager.h"
#include <algorithm>


namespace TDEngine2
{
	CSceneManager::CSceneManager() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CSceneManager::Init(TPtr<IFileSystem> pFileSystem, TPtr<IWorld> pWorld, const TSceneManagerSettings& settings)
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

		mIsInitialized = true;

		return _onPostInit();
	}

	E_RESULT_CODE CSceneManager::_onFreeInternal()
	{
		E_RESULT_CODE result = RC_OK;

		for (IScene* pCurrScene : mpScenes)
		{
			result = result | pCurrScene->Free();
		}

		mpScenes.clear();

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
		auto iter = std::find_if(mpScenes.cbegin(), mpScenes.cend(), [&sceneName](const IScene* pScene) { return pScene->GetName() == sceneName; });
		if (iter != mpScenes.cend())
		{
			return Wrench::TOkValue<TSceneId>(static_cast<TSceneId>(std::distance(mpScenes.cbegin(), iter)));
		}

		E_RESULT_CODE result = RC_OK;

		IScene* pScene = TDEngine2::CreateScene(mpWorld.Get(), sceneName, scenePath, mpScenes.empty(), result); // \todo Add check up for a main scene flag

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

		if (!onResultCallback)
		{
			return;
		}

		IJobManager* pJobManager = mpFileSystem->GetJobManager();
		if (!pJobManager)
		{
			onResultCallback(Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS));
			return;
		}

		pJobManager->SubmitJob(std::function<void()>([this, scenePath, onResultCallback, pJobManager]()
		{
			const std::string& sceneName = mpFileSystem->ExtractFilename(scenePath);

			// \note If there is loaded scene then just return its handle
			auto iter = std::find_if(mpScenes.cbegin(), mpScenes.cend(), [&scenePath](const IScene* pScene) { return pScene->GetScenePath() == scenePath; });
			if (iter != mpScenes.cend())
			{
				/// \note This callback should be executed in the main thread
				pJobManager->ExecuteInMainThread([onResultCallback, sceneId = static_cast<TSceneId>(std::distance(mpScenes.cbegin(), iter))] 
					{
						onResultCallback(Wrench::TOkValue<TSceneId>(sceneId));
					});

				return;
			}

			E_RESULT_CODE result = RC_OK;

			IScene* pScene = TDEngine2::CreateScene(mpWorld.Get(), sceneName, scenePath, mpScenes.empty(), result); // \todo Add check up for a main scene flag

			if (RC_OK != result || !pScene)
			{
				/// \note This callback should be executed in the main thread
				pJobManager->ExecuteInMainThread([onResultCallback, result] { onResultCallback(Wrench::TErrValue<E_RESULT_CODE>(result)); });

				return;
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
				/// \note This callback should be executed in the main thread
				pJobManager->ExecuteInMainThread([onResultCallback, result] { onResultCallback(Wrench::TErrValue<E_RESULT_CODE>(result)); });

				return;
			}

			/// \note This callback should be executed in the main thread
			pJobManager->ExecuteInMainThread([onResultCallback, this, sceneName, pScene] { onResultCallback(_registerSceneInternal(sceneName, pScene)); });
		}));
	}

	E_RESULT_CODE CSceneManager::UnloadScene(TSceneId id)
	{
		if (auto findSceneResult = GetScene(id))
		{
			if (IScene* pScene = findSceneResult.Get())
			{
				E_RESULT_CODE result = pScene->Free();
				result = result | _unregisterSceneInternal(id);

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

		auto iter = std::find_if(mpScenes.cbegin(), mpScenes.cend(), [&id](const IScene* pScene) { return pScene->GetName() == id; });
		if (iter == mpScenes.cend())
		{
			return TSceneId::Invalid;
		}

		return static_cast<TSceneId>(std::distance(mpScenes.cbegin(), iter));
	}

	TPtr<IWorld> CSceneManager::GetWorld() const
	{
		return mpWorld;
	}

	const CSceneManager::TScenesArray& CSceneManager::GetLoadedScenes() const
	{
		std::lock_guard<std::mutex> lock(mMutex);

		return mpScenes;
	}

	TResult<TSceneId> CSceneManager::_createInternal(const std::string& name)
	{
		E_RESULT_CODE result = RC_OK;

		IScene* pScene = TDEngine2::CreateScene(mpWorld.Get(), name, Wrench::StringUtils::GetEmptyStr(), mpScenes.empty(), result);

		if (RC_OK != result)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		return _registerSceneInternal(name, pScene);
	}

	TResult<TSceneId> CSceneManager::_registerSceneInternal(const std::string& name, IScene* pScene)
	{
		auto iter = std::find_if(mpScenes.cbegin(), mpScenes.cend(), [&name](const IScene* pSceneEntity)
		{
			return pSceneEntity->GetName() == name;
		});

		if (iter != mpScenes.cend())
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		const TSceneId id = static_cast<TSceneId>(mpScenes.size());

		mpScenes.push_back(pScene);

		return Wrench::TOkValue<TSceneId>(id);
	}

	E_RESULT_CODE CSceneManager::_unregisterSceneInternal(TSceneId id)
	{
		if (TSceneId::Invalid == id)
		{
			return RC_INVALID_ARGS;
		}

		mpScenes.erase(mpScenes.cbegin() + static_cast<U32>(id));

		return RC_OK;
	}

	TDE2_API E_RESULT_CODE CSceneManager::_onPostInit()
	{
		// \note Load main scene in synchronous fashion
		if (auto openSceneFileResult = mpFileSystem->Open<IYAMLFileReader>(mSettings.mMainScenePath)) // \note File exists, so we load it
		{
			auto mainSceneLoadingResult = _loadSceneInternal(mSettings.mMainScenePath);
			if (mainSceneLoadingResult.HasError())
			{
				return mainSceneLoadingResult.GetError();
			}
		}

		const std::string& sceneName = mpFileSystem->ExtractFilename(mSettings.mMainScenePath);
		
		// \create a new empty scene		
		auto mainSceneCreationResult = _createInternal(sceneName);
		if (mainSceneCreationResult.HasError())
		{
			return mainSceneCreationResult.GetError();
		}

		return RC_OK;
	}


	TDE2_API ISceneManager* CreateSceneManager(TPtr<IFileSystem> pFileSystem, TPtr<IWorld> pWorld, const TSceneManagerSettings& settings, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISceneManager, CSceneManager, result, pFileSystem, pWorld, settings);
	}
}