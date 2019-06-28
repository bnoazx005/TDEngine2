#include "./../../../include/core/CBaseFileSystem.h"
#include "./../../../include/core/IFile.h"
#include "./../../../include/platform/CTextFileReader.h"
#include "./../../../include/utils/CFileLogger.h"
#include "./../../../include/core/IJobManager.h"
#include <algorithm>
#if _HAS_CXX17
#include <filesystem>
#include <fstream>
#else
#include <fstream>
#endif


namespace TDEngine2
{
	std::string CBaseFileSystem::mInvalidPath = "";

	CBaseFileSystem::CBaseFileSystem() :
		CBaseObject(), mpJobManager(nullptr)
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
		std::lock_guard<std::mutex> lock(mMutex);

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
		std::lock_guard<std::mutex> lock(mMutex);

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
	
	E_RESULT_CODE CBaseFileSystem::CloseFile(const std::string& filename)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (!mIsInitialized)
		{
			return RC_FAIL;
		}
		
		return _closeFile(filename);
	}

	E_RESULT_CODE CBaseFileSystem::CloseFile(TFileEntryId fileId)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		if (fileId == InvalidFileEntryId)
		{
			return RC_INVALID_ARGS;
		}

		TResult<IFile*> filePointer = mActiveFiles[fileId];

		if (filePointer.HasError())
		{
			return filePointer.GetError();
		}

		IFile* pFileEntry = filePointer.Get();

		return _closeFile(pFileEntry->GetFilename());
	}

	E_RESULT_CODE CBaseFileSystem::CloseAllFiles()
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		IFile* pCurrFile = nullptr;

		for (U32 i = 0; i < mActiveFiles.GetSize(); ++i)
		{
			pCurrFile = mActiveFiles[i].Get();

			if (!pCurrFile) /// if pCurrFile is nullptr then it has already closed manually
			{
				continue;
			}

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

	void CBaseFileSystem::SetJobManager(IJobManager* pJobManager)
	{
		mpJobManager = pJobManager;
	}

	IJobManager* CBaseFileSystem::GetJobManager() const
	{
		return mpJobManager;
	}

	bool CBaseFileSystem::IsStreamingEnabled() const
	{
		return mpJobManager != nullptr;
	}

	TFileEntryId CBaseFileSystem::_registerFileEntry(IFile* pFileEntry)
	{
		U32 fileEntryId = mActiveFiles.Add(pFileEntry);

		mFilesMap[pFileEntry->GetFilename()] = fileEntryId;
		
		return fileEntryId;
	}

	TResult<TFileEntryId> CBaseFileSystem::_openFile(U32 typeId, const std::string& filename, bool createIfDoesntExist)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (!mIsInitialized)
		{
			return TErrorValue<E_RESULT_CODE>(RC_FAIL);
		}

		if (!_isPathValid(filename))
		{
			return TErrorValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		TFileFactoriesRegistry::const_iterator fileFactoryIter = mFileFactoriesMap.find(typeId);

		if (fileFactoryIter == mFileFactoriesMap.cend())
		{
			return TErrorValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		///try to find the file's factory
		TCreateFileCallback pFileFactory = mFileFactories[(*fileFactoryIter).second].Get();

		if (!pFileFactory)
		{
			return TErrorValue<E_RESULT_CODE>(RC_FAIL);
		}

		/// create a file's entry on a disk if doesn't exist but a user asks for it
		if (!FileExists(filename) && createIfDoesntExist)
		{
			LOG_MESSAGE("[File System] A new file was created (TypeID : " + std::to_string(typeId) + "; filename: " + filename + ")");

			_createNewFile(filename);
		}

		E_RESULT_CODE result = RC_OK;

		IFile* pNewFileInstance = pFileFactory(this, filename, result);

		if (result != RC_OK)
		{
			return TErrorValue<E_RESULT_CODE>(result);
		}

		TFileEntryId newFileEntryId = _registerFileEntry(pNewFileInstance);
		
		LOG_MESSAGE("[File System] A new file descriptor was created by the manager (" + filename + ")");

		return TOkValue<TFileEntryId>(newFileEntryId);
	}
	
	E_RESULT_CODE CBaseFileSystem::_registerFileFactory(U32 typeId, TCreateFileCallback pCreateFileCallback)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (!pCreateFileCallback) /// \todo check typeId equals to InvalidTypeId
		{
			return RC_INVALID_ARGS;
		}

		TFileFactoriesRegistry::const_iterator factoryIter = mFileFactoriesMap.find(typeId);

		if (factoryIter != mFileFactoriesMap.cend())
		{
			return RC_FAIL;
		}
		
		mFileFactoriesMap[typeId] = mFileFactories.Add(pCreateFileCallback);
		
		LOG_MESSAGE("[File System] A new file factory was registred by the manager (TypeID : " + std::to_string(typeId) + ")");

		return RC_OK;
	}

	E_RESULT_CODE CBaseFileSystem::_unregisterFileFactory(U32 typeId)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		TFileFactoriesRegistry::const_iterator fileFactoryEntryIter = mFileFactoriesMap.find(typeId);

		if (fileFactoryEntryIter == mFileFactoriesMap.cend())
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = mFileFactories.RemoveAt(fileFactoryEntryIter->second);

		if (result != RC_OK)
		{
			return result;
		}

		LOG_MESSAGE("[File System] The folowing file factory was unregistred by the manager (TypeID : " + std::to_string(typeId) + ")");

		return RC_OK;
	}

	IFile* CBaseFileSystem::_getFile(TFileEntryId fileId)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (fileId == InvalidFileEntryId)
		{
			return nullptr;
		}

		TResult<IFile*> filePointer = mActiveFiles[fileId];

		if (filePointer.HasError())
		{
			return nullptr;
		}

		IFile* pFile = filePointer.Get();

		/// to prevent deletion of a file by one thread when it can be used in another we need to increment its internal counter's value
		/// later used should Close it. This action will decrement internal counter's value and may destroy the file.
		if (!pFile->IsParentThread()) 
		{
			pFile->AddRef();
		}

		return pFile;
	}

	E_RESULT_CODE CBaseFileSystem::_closeFile(const std::string& filename)
	{
		auto entryHashIter = mFilesMap.find(filename);

		if (entryHashIter == mFilesMap.cend())
		{
			return RC_FAIL;
		}

		TFileEntryId fileEntryId = (*entryHashIter).second;

		mFilesMap.erase(entryHashIter);

		mActiveFiles.RemoveAt(fileEntryId);

		LOG_MESSAGE("[File System] Existing file descriptor was closed (" + filename + ")");

		return RC_OK;
	}

	void CBaseFileSystem::_createNewFile(const std::string& filename)
	{
		std::ofstream newFileInstance(filename.c_str());
		newFileInstance.close();
	}
}