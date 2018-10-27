#include "./../../../include/core/CBaseFileSystem.h"
#include "./../../../include/core/IFile.h"
#include "./../../../include/platform/CTextFileReader.h"
#include "./../../../include/utils/CFileLogger.h"
#include <algorithm>
#if _HAS_CXX17
#include <filesystem>
#else
#include <fstream>
#endif


namespace TDEngine2
{
	std::string CBaseFileSystem::mInvalidPath = "";

	CBaseFileSystem::CBaseFileSystem() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CBaseFileSystem::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		LOG_MESSAGE("[File System] The file system  was successfully initialized");

		return _onInit();
	}

	E_RESULT_CODE CBaseFileSystem::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		result = CloseAllFiles();

		if (result != RC_OK)
		{
			return result;
		}

		mIsInitialized = false;

		delete this;

		LOG_MESSAGE("[File System] The file system  was successfully destroyed");

		return RC_OK;
	}

	E_RESULT_CODE CBaseFileSystem::Mount(const std::string& path, const std::string& aliasPath)
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		std::string unifiedPath = _unifyPathView(path);
		std::string unifiedAliasPath = _unifyPathView(aliasPath, true);

		if (unifiedPath.empty() || unifiedAliasPath.empty())
		{
			return RC_INVALID_ARGS;
		}

		auto existedCopyIter = mVirtualPathsMap.find(unifiedAliasPath);

		if (existedCopyIter != mVirtualPathsMap.cend())
		{
			return RC_FAIL;
		}

		mVirtualPathsMap[unifiedAliasPath] = unifiedPath;

		LOG_MESSAGE("[File System] A new virtual path was mounted (" + unifiedPath + " -> " + unifiedAliasPath + ")");

		return RC_OK;
	}

	E_RESULT_CODE CBaseFileSystem::Unmount(const std::string& aliasPath)
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		std::string unifiedAliasPath = _unifyPathView(aliasPath);

		auto existedEntryIter = mVirtualPathsMap.find(unifiedAliasPath);

		if (existedEntryIter == mVirtualPathsMap.cend())
		{
			return RC_FAIL;
		}

		mVirtualPathsMap.erase(existedEntryIter);

		LOG_MESSAGE("[File System] Existing virtual path was unmounted (" + unifiedAliasPath + ")");

		return RC_OK;
	}
	
	E_RESULT_CODE CBaseFileSystem::CloseFile(IFile* pFile)
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pFile)
		{
			return RC_INVALID_ARGS;
		}

		std::string filename = pFile->GetFilename();

		auto entryHashIter = mFilesMap.find(filename);

		if (entryHashIter == mFilesMap.cend() ||	// there is no file's entry within the registry or its internal descriptor is still opened
			pFile->IsOpen())
		{
			return RC_FAIL;
		}

		U32 hashValue = (*entryHashIter).second;

		mFilesMap.erase(entryHashIter);

		mActiveFiles[hashValue] = nullptr;

		LOG_MESSAGE("[File System] Existing file descriptor was closed (" + filename + ")");

		return RC_OK;
	}

	E_RESULT_CODE CBaseFileSystem::CloseAllFiles()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		for (IFile* pCurrFile : mActiveFiles)
		{
			result = pCurrFile->Close();

			if (result != RC_OK)
			{
				return result;
			}
		}

		return RC_OK;
	}

	bool CBaseFileSystem::FileExists(const std::string& filename) const
	{
#if _HAS_CXX17
		return std::experimental::filesystem::exists(filename);
#else
		std::ifstream file(filename);

		bool exists = file.good();

		file.close();

		return exists;
#endif
	}

	std::string CBaseFileSystem::GetCurrDirectory() const
	{
#if _HAS_CXX17
		return std::experimental::filesystem::current_path().string();
#else
		return ".";
#endif
	}

	E_ENGINE_SUBSYSTEM_TYPE CBaseFileSystem::GetType() const
	{
		return EST_FILE_SYSTEM;
	}

	E_RESULT_CODE CBaseFileSystem::_registerFileEntry(IFile* pFileEntry)
	{
		U32 hashValue = mActiveFiles.size();

		mActiveFiles.push_back(pFileEntry);

		mFilesMap[pFileEntry->GetFilename()] = hashValue;
		
		return RC_OK;
	}

	IFile* CBaseFileSystem::_createFile(U32 typeId, const std::string& filename, E_RESULT_CODE& result)
	{
		if (!mIsInitialized)
		{
			result = RC_FAIL;

			return nullptr;
		}

		if (!_isPathValid(filename))
		{
			result = RC_INVALID_ARGS;

			return nullptr;
		}

		TFileFactoriesRegistry::const_iterator fileFactoryIter = mFileFactoriesMap.find(typeId);

		if (fileFactoryIter == mFileFactoriesMap.cend())
		{
			result = RC_INVALID_ARGS;

			return nullptr;
		}

		///try to find the file's factory
		TCreateFileCallback pFileFactory = mFileFactories[(*fileFactoryIter).second];

		if (!pFileFactory)
		{
			result = RC_FAIL;

			return nullptr;
		}

		IFile* pNewFileInstance = pFileFactory(this, filename, result);

		if (result != RC_OK)
		{
			return nullptr;
		}

		result = _registerFileEntry(pNewFileInstance);

		if (result != RC_OK)
		{
			return nullptr;
		}

		LOG_MESSAGE("[File System] A new file descriptor was created by the manager (" + filename + ")");

		return pNewFileInstance;
	}
	
	E_RESULT_CODE CBaseFileSystem::_registerFileFactory(U32 typeId, TCreateFileCallback pCreateFileCallback)
	{
		if (!pCreateFileCallback) /// \todo check typeId equals to InvalidTypeId
		{
			return RC_INVALID_ARGS;
		}

		TFileFactoriesRegistry::const_iterator factoryIter = mFileFactoriesMap.find(typeId);

		if (factoryIter != mFileFactoriesMap.cend())
		{
			return RC_FAIL;
		}

		if (mFileFactoriesFreeSlots.empty())
		{
			U32 nextSlotHash = mFileFactories.size() + 1;

			mFileFactories.push_back(pCreateFileCallback);

			mFileFactoriesMap[typeId] = nextSlotHash - 1;

			LOG_MESSAGE("[File System] A new file factory was registred by the manager (TypeID : " + std::to_string(typeId) + ")");

			return RC_OK;
		}

		U32 nextSlotHash = mFileFactoriesFreeSlots.front() + 1;

		mFileFactoriesFreeSlots.pop_front();

		mFileFactories[nextSlotHash] = pCreateFileCallback;

		mFileFactoriesMap[typeId] = nextSlotHash - 1;

		LOG_MESSAGE("[File System] A new file factory was registred by the manager");

		return RC_OK;
	}

	E_RESULT_CODE CBaseFileSystem::_unregisterFileFactory(U32 typeId)
	{
		TFileFactoriesRegistry::const_iterator fileFactoryEntryIter = mFileFactoriesMap.find(typeId);

		if (fileFactoryEntryIter == mFileFactoriesMap.cend())
		{
			return RC_FAIL;
		}

		U32 hashValue = (*fileFactoryEntryIter).second;

		mFileFactories[hashValue - 1] = nullptr;

		mFileFactoriesFreeSlots.push_back(hashValue - 1);

		mFileFactoriesMap.erase(fileFactoryEntryIter);

		LOG_MESSAGE("[File System] Existing file factory was unregistred by the manager");

		return RC_OK;
	}
}