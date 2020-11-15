#include "../../include/scene/CSceneManager.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/scene/CScene.h"
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
		return Wrench::TErrValue<E_RESULT_CODE>(RC_NOT_IMPLEMENTED_YET);
	}

	void CSceneManager::LoadSceneAsync(const std::string& scenePath, const TLoadSceneCallback& onResultCallback)
	{
		// \note Create empty scene
		// \note Deserialize its data based on archive at scenePath
	}

	E_RESULT_CODE CSceneManager::UnloadScene(TSceneId id)
	{
		return RC_NOT_IMPLEMENTED_YET;
	}

	TResult<TSceneId> CSceneManager::CreateScene(const std::string& name)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _createInternal(name);
	}

	E_RESULT_CODE CSceneManager::RemoveScene(TSceneId id)
	{
		return RC_NOT_IMPLEMENTED_YET;
	}

	TResult<IScene*> CSceneManager::GetScene(TSceneId id) const
	{
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

		auto iter = std::find_if(mpScenes.cbegin(), mpScenes.cend(), [&name](const IScene* pScene)
		{
			return pScene->GetName() == name;
		});

		if (iter != mpScenes.cend())
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		const TSceneId id = static_cast<TSceneId>(mpScenes.size());

		mpScenes.push_back(pScene);

		return Wrench::TOkValue<TSceneId>(id);
	}


	TDE2_API ISceneManager* CreateSceneManager(IFileSystem* pFileSystem, IWorld* pWorld, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISceneManager, CSceneManager, result, pFileSystem, pWorld);
	}
}