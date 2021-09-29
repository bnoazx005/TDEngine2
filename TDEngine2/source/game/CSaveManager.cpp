#include "../../include/game/CSaveManager.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/core/IFile.h"
#include "../../include/game/CSaveData.h"
#include <stringUtils.hpp>
#include <deferOperation.hpp>


namespace TDEngine2
{
	CSaveManager::CSaveManager():
		CBaseObject()
	{
	}

	E_RESULT_CODE CSaveManager::Init(const TSaveManagerParametersDesc& desc)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!desc.mpFileSystem)
		{
			return RC_INVALID_ARGS;
		}

		std::lock_guard<std::mutex> lock(mMutex);

		mpFileSystem = desc.mpFileSystem;

		E_RESULT_CODE result = _scanAndLoadSaveFiles(desc.mUserDataDirectoryPath);
		if (result != RC_OK)
		{
			return result;
		}

		LOG_MESSAGE("[Save Manager] The save manager system was successfully initialized");

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CSaveManager::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;
		
		--mRefCounter;

		if (!mRefCounter)
		{
			result = _freeSaveGamesObjects();

			mIsInitialized = false;
			delete this;
		}

		return result;
	}

	TSaveDataId CSaveManager::CreateSaveData(const std::string& saveId, const std::string& path, const std::function<bool()>& onOverwriteFile)
	{
		TSaveDataId id = FindSaveData(saveId);
		if (TSaveDataId::Invalid != id)
		{
			return id;
		}

		std::lock_guard<std::mutex> lock(mMutex);

		ISaveData* pNewSaveData = nullptr;

		// \note Create in-memory representation
		E_RESULT_CODE result = RC_OK;
		{
			id = static_cast<TSaveDataId>(mpSaves.size());

			pNewSaveData = ::TDEngine2::CreateSaveData(result);
			result = result | pNewSaveData->SetId(saveId);

			mpSaves.insert({ id, pNewSaveData });
		}

		// \note Write it down the file for the save
		if (mpFileSystem->FileExists(path) && onOverwriteFile && !onOverwriteFile())
		{
			return id;
		}

		if (auto openFileResult = mpFileSystem->Open<IYAMLFileWriter>(path, true))
		{
			IYAMLFileWriter* pFileWriter = mpFileSystem->Get<IYAMLFileWriter>(openFileResult.Get()); // \todo Refactor this to choose between binary or YAML format

			// \note Proceed to read the file, because it's really save data
			defer([pFileWriter] { pFileWriter->Close(); });

			E_RESULT_CODE result = RC_OK;

			pNewSaveData->Save(pFileWriter);
		}

		return id;
	}

	E_RESULT_CODE CSaveManager::RemoveSaveData(TSaveDataId id)
	{
		ISaveData* pSaveData = GetSaveData(id);
		if (!pSaveData)
		{
			return RC_FAIL;
		}

		const std::string savePath = pSaveData->GetPath();

		E_RESULT_CODE result = pSaveData->Free();
		if (RC_OK != result)
		{
			return result;
		}

		mpSaves.erase(id);

		if (!mpFileSystem->FileExists(savePath))
		{
			return RC_FILE_NOT_FOUND;
		}

		if (RC_OK != (result = mpFileSystem->RemoveFile(savePath)))
		{
			return result;
		}

		return RC_OK;
	}

	TSaveDataId CSaveManager::FindSaveData(const std::string& saveId) const
	{
		std::lock_guard<std::mutex> lock(mMutex);

		auto iter = std::find_if(mpSaves.cbegin(), mpSaves.cend(), [&saveId](auto&& entity) { return entity.second->GetId() == saveId; });
		return iter == mpSaves.cend() ? TSaveDataId::Invalid : iter->first;
	}

	ISaveData* CSaveManager::GetSaveData(TSaveDataId id) const
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (id == TSaveDataId::Invalid)
		{
			return nullptr;
		}

		auto iter = mpSaves.find(id);
		if (iter == mpSaves.cend())
		{
			return nullptr;
		}

		return mpSaves.at(id);
	}

	E_ENGINE_SUBSYSTEM_TYPE CSaveManager::GetType() const
	{
		return ISaveManager::GetTypeID();
	}

	E_RESULT_CODE CSaveManager::_scanAndLoadSaveFiles(const std::string& path)
	{
		if (path.empty())
		{
			return RC_INVALID_ARGS;
		}

		auto&& filesList = mpFileSystem->GetFilesListAtDirectory(path);

		// \note Iterate over files and check whether they are saves or not
		for (const std::string& currFilepath : filesList)
		{
			if (!Wrench::StringUtils::EndsWith(currFilepath, "savedata"))
			{
				continue;
			}

			if (auto openFileResult = mpFileSystem->Open<IYAMLFileReader>(currFilepath))
			{
				IYAMLFileReader* pFileReader = mpFileSystem->Get<IYAMLFileReader>(openFileResult.Get()); // \todo Refactor this to choose between binary or YAML format

				// \note Proceed to read the file, because it's really save data
				defer([pFileReader] { pFileReader->Close(); });

				E_RESULT_CODE result = RC_OK;

				ISaveData* pSaveData = ::TDEngine2::CreateSaveData(result);
				if (result != RC_OK)
				{
					continue;
				}

				pSaveData->Load(pFileReader);

				mpSaves.insert({ static_cast<TSaveDataId>(mpSaves.size()), pSaveData });
			}
		}

		return RC_OK;
	}

	E_RESULT_CODE CSaveManager::_freeSaveGamesObjects()
	{
		E_RESULT_CODE result = RC_OK;

		for (auto&& currSaveDataEntity : mpSaves)
		{
			result = result | currSaveDataEntity.second->Free();
		}

		return result;
	}


	TDE2_API ISaveManager* CreateSaveManager(const TSaveManagerParametersDesc& desc, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISaveManager, CSaveManager, result, desc);
	}
}