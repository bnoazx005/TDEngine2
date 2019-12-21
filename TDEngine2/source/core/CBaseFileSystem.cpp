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

		std::string unifiedAliasPath = _unifyPathView(aliasPath, true);

		auto existedEntryIter = mVirtualPathsMap.find(unifiedAliasPath);

		if (existedEntryIter == mVirtualPathsMap.cend())
		{
			return RC_FAIL;
		}

		mVirtualPathsMap.erase(existedEntryIter);

		LOG_MESSAGE("[File System] Existing virtual path was unmounted (" + unifiedAliasPath + ")");

		return RC_OK;
	}
	
	std::string CBaseFileSystem::ResolveVirtualPath(const std::string& path, bool isDirectory) const
	{
		std::string unifiedPath = _unifyPathView(path, CStringUtils::StartsWith(path, "vfs:"), isDirectory);

		if (unifiedPath.empty() || (unifiedPath.size() > 1 && !CStringUtils::StartsWith(unifiedPath, "vfs:")))
		{
			return unifiedPath; // this case includes all real paths, single files
		}

		const C8 pathSeparator = GetPathSeparatorChar();

		if (unifiedPath == _getVirtualPathPrefixStr()) // replace vfs:// virtual root directory with current directory of an application
		{
			return this->GetCurrDirectory();
		}

		U32 prevDirectoryPos = 0;
		U32 currDirectoryPos = 0;
		U32 uncheckedPathPos = 0;

		std::string currPath, lastMatchPart;

		TVirtualPathsMap::const_iterator physicalPathIter;

		/// select the greatest coincidental part of the path
		while ((currDirectoryPos = unifiedPath.find_first_of(GetPathSeparatorChar(), prevDirectoryPos)) != std::string::npos)
		{
			prevDirectoryPos = currDirectoryPos + 1;

			currPath = unifiedPath.substr(0, prevDirectoryPos);

			physicalPathIter = mVirtualPathsMap.find(currPath);

			if (physicalPathIter != mVirtualPathsMap.cend())
			{
				lastMatchPart = (*physicalPathIter).second;

				uncheckedPathPos = currDirectoryPos;
			}
		}

		// check special case when last part of a path doesn't contains an extension's declaration like so \\foo
		// in this situation we should try to interpretate it as a possible virtual path
		if ((currPath = unifiedPath.substr(prevDirectoryPos - 1) + GetPathSeparatorChar()).find('.') == std::string::npos)
		{
			// the part could be a directory and a filename as well

			physicalPathIter = mVirtualPathsMap.find(currPath);

			if (physicalPathIter != mVirtualPathsMap.cend())
			{
				return (*physicalPathIter).second;
			}
		}

		return lastMatchPart + unifiedPath.substr(uncheckedPathPos + 1, unifiedPath.length() - uncheckedPathPos);
	}

	std::string CBaseFileSystem::_unifyPathView(const std::string& path, bool isVirtualPath, bool isDirectory) const
	{
		if (path.empty())
		{
			if (isVirtualPath)
			{
				return _getVirtualPathPrefixStr();
			}

			return mInvalidPath;
		}

		std::string unifiedPath = path;

		std::replace(unifiedPath.begin(), unifiedPath.end(), GetAltPathSeparatorChar(), GetPathSeparatorChar());

		// check up is the path valid
		if (!_isPathValid(unifiedPath, isVirtualPath))
		{
			return mInvalidPath;
		}
		
		// \note add path separator at the end of the path
		if ((unifiedPath.back() != GetPathSeparatorChar()) && isDirectory)
		{
			unifiedPath.push_back(GetPathSeparatorChar());
		}

		if (isVirtualPath)
		{
			// \note remove any path separator from the beginning of unifiedPath string
			if (unifiedPath.front() == GetPathSeparatorChar())
			{
				unifiedPath.erase(unifiedPath.begin());
			}

			static const std::string vfsPathPrefixStr = _getVirtualPathPrefixStr();
			return CStringUtils::StartsWith(unifiedPath, vfsPathPrefixStr) ? unifiedPath : vfsPathPrefixStr + unifiedPath;
		}

		return unifiedPath;
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

	std::string CBaseFileSystem::GetExtension(const std::string& path) const
	{
#if _HAS_CXX17
		return std::experimental::filesystem::path(path).extension().string();
#else
		std::string::size_type pos = path.find_last_of('.');

		return (pos != std::string::npos) ? path.substr(pos) : "";
#endif
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

	std::string CBaseFileSystem::_getVirtualPathPrefixStr() const
	{
		static std::string virtualPathPrefix = CStringUtils::Format("vfs:{0}{0}", GetPathSeparatorChar());
		return virtualPathPrefix;
	}
}