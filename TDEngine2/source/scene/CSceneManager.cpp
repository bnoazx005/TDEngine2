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

	E_RESULT_CODE CSceneManager::Init(IFileSystem* pFileSystem, IWorld* pWorld)
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

		mpFileSystem = pFileSystem;
		mpWorld = pWorld;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CSceneManager::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		--mRefCounter;

		E_RESULT_CODE result = RC_OK;

		if (!mRefCounter)
		{
			for (IScene* pCurrScene : mpScenes)
			{
				result = result | pCurrScene->Free();
			}

			mpScenes.clear();

			mIsInitialized = false;
			delete this;
		}

		return result;
	}

	TResult<TSceneId> CSceneManager::LoadScene(const std::string& scenePath)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		const std::string& sceneName = mpFileSystem->ExtractFilename(scenePath);

		// \note If there is loaded scene then just return its handle
		auto iter = std::find_if(mpScenes.cbegin(), mpScenes.cend(), [&sceneName](const IScene* pScene) { return pScene->GetName() == sceneName; });
		if (iter != mpScenes.cend())
		{
			return Wrench::TOkValue<TSceneId>(static_cast<TSceneId>(std::distance(mpScenes.cbegin(), iter)));
		}

		E_RESULT_CODE result = RC_OK;

		IScene* pScene = TDEngine2::CreateScene(mpWorld, sceneName, scenePath, false, result); // \todo Add check up for a main scene flag

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

		pJobManager->SubmitJob(std::function<void()>([this, scenePath, onResultCallback]()
		{
			const std::string& sceneName = mpFileSystem->ExtractFilename(scenePath);

			// \note If there is loaded scene then just return its handle
			auto iter = std::find_if(mpScenes.cbegin(), mpScenes.cend(), [&sceneName](const IScene* pScene) { return pScene->GetName() == sceneName; });
			if (iter != mpScenes.cend())
			{
				onResultCallback(Wrench::TOkValue<TSceneId>(static_cast<TSceneId>(std::distance(mpScenes.cbegin(), iter))));
			}

			E_RESULT_CODE result = RC_OK;

			IScene* pScene = TDEngine2::CreateScene(mpWorld, sceneName, scenePath, false, result); // \todo Add check up for a main scene flag

			if (RC_OK != result || !pScene)
			{
				onResultCallback(Wrench::TErrValue<E_RESULT_CODE>(result));
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
				onResultCallback(Wrench::TErrValue<E_RESULT_CODE>(result));
				return;
			}

			onResultCallback(_registerSceneInternal(sceneName, pScene));
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

	TResult<TSceneId> CSceneManager::_createInternal(const std::string& name)
	{
		E_RESULT_CODE result = RC_OK;

		IScene* pScene = TDEngine2::CreateScene(mpWorld, name, Wrench::StringUtils::GetEmptyStr(), mpScenes.empty(), result);

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


	TDE2_API ISceneManager* CreateSceneManager(IFileSystem* pFileSystem, IWorld* pWorld, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISceneManager, CSceneManager, result, pFileSystem, pWorld);
	}
}