#include "../../../include/core/CBaseFileSystem.h"
#include "../../../include/core/IFile.h"
#include "../../../include/platform/CTextFileReader.h"
#include "../../../include/utils/CFileLogger.h"
#include "../../../include/core/IJobManager.h"
#include "../../../include/platform/IOStreams.h"
#include "../../../include/platform/MountableStorages.h"
#include "stringUtils.hpp"
#include <algorithm>
#include <experimental/filesystem>
#if _HAS_CXX17
#include <filesystem>
#include <fstream>
#else
#include <fstream>
#endif


namespace TDEngine2
{
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

		mIsInitialized = false;
		delete this;

		LOG_MESSAGE("[File System] The file system  was successfully destroyed");

		return RC_OK;
	}

	E_RESULT_CODE CBaseFileSystem::MountPhysicalPath(const std::string& path, const std::string& aliasPath, U16 relativePriority)
	{
		E_RESULT_CODE result = RC_OK;

		IMountableStorage* pStorage = CreatePhysicalFilesStorage(this, _normalizePathView(path), result);
		if (result != RC_OK || !pStorage)
		{
			return result;
		}

		return _mountInternal(aliasPath, pStorage, relativePriority);
	}

	E_RESULT_CODE CBaseFileSystem::MountPackage(const std::string& path, const std::string& aliasPath)
	{
		E_RESULT_CODE result = RC_OK;

		IMountableStorage* pStorage = CreatePackageFilesStorage(this, _normalizePathView(path, false), result);
		if (result != RC_OK || !pStorage)
		{
			return result;
		}

		return _mountInternal(aliasPath, pStorage, 0);
	}

	E_RESULT_CODE CBaseFileSystem::Unmount(const std::string& aliasPath)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		const std::string unifiedAliasPath = _normalizePathView(aliasPath);

		auto existedEntryIter = std::find_if(mMountedStorages.cbegin(), mMountedStorages.cend(), [&unifiedAliasPath](auto&& entry) { return entry.mAliasPath == unifiedAliasPath; });
		if (existedEntryIter == mMountedStorages.cend())
		{
			LOG_WARNING(Wrench::StringUtils::Format("[File System] Try to unmount unexisting storage, alias: {0}", unifiedAliasPath));
			return RC_FAIL;
		}

		if (auto pStorage = existedEntryIter->mpStorage)
		{
			E_RESULT_CODE result = pStorage->Free();
			if (RC_OK != result)
			{
				return result;
			}
		}		

		mMountedStorages.erase(existedEntryIter);

		LOG_MESSAGE("[File System] Existing virtual path was unmounted (" + unifiedAliasPath + ")");

		return RC_OK;
	}

	std::string CBaseFileSystem::ResolveVirtualPath(const std::string& path, bool isDirectory) const
	{
		std::lock_guard<std::mutex> lock(mMutex);

		const std::string filePath = _normalizePathView(path, isDirectory);

		for (auto&& currMountPointEntry : mMountedStorages)
		{
			const std::string& alias = _normalizePathView(currMountPointEntry.mAliasPath, true);

			/// \note Try to open file with current storage, but it could fail
			if (Wrench::StringUtils::StartsWith(filePath, alias))
			{
				return _normalizePathView(_resolveVirtualPathInternal(currMountPointEntry, filePath, isDirectory), isDirectory);
			}
		}

		return path;
	}

	std::string CBaseFileSystem::ExtractFilename(const std::string& path) const
	{
		return std::experimental::filesystem::path(path).filename().string();
	}

	E_RESULT_CODE CBaseFileSystem::_mountInternal(const std::string& aliasPath, IMountableStorage* pStorage, U16 realtivePriority)
	{
		const std::string unifiedAliasPath = _normalizePathView(aliasPath);

		{
			std::lock_guard<std::mutex> lock(mMutex);
			
			if (unifiedAliasPath.empty() || !pStorage)
			{
				return RC_INVALID_ARGS;
			}

			E_RESULT_CODE result = RC_OK;

			auto iter = std::find_if(mMountedStorages.begin(), mMountedStorages.end(), [&unifiedAliasPath](auto&& entry) { return entry.mAliasPath == unifiedAliasPath; });
			if (iter != mMountedStorages.end())
			{
				LOG_WARNING(Wrench::StringUtils::Format("[File System] Replace existing mounted storage with a new one (mount path: {0})", unifiedAliasPath));

				// \note Remove previous storage
				IMountableStorage* pExistedStorage = iter->mpStorage;
				TDE2_ASSERT(pExistedStorage);

				if (pExistedStorage)
				{
					if (RC_OK != (result = pExistedStorage->Free()))
					{
						return result;
					}
				}

				// \note Register the new one
				iter->mpStorage = pStorage;

				return RC_OK;
			}

			// \note Insert a new mounting storage based on its priority, type and path's order
			if (mMountedStorages.empty())
			{
				mMountedStorages.push_back(TMountedStorageInfo { pStorage, aliasPath });
			}
			else
			{
				bool isInserted = false;

				for (auto iter = mMountedStorages.cbegin(); iter != mMountedStorages.cend(); ++iter)
				{
					if (iter->mpStorage->GetPriority() >= (pStorage->GetPriority() + realtivePriority))
					{
						mMountedStorages.insert(iter, TMountedStorageInfo { pStorage, aliasPath });
						isInserted = true;
						break;
					}
				}

				if (!isInserted)
				{
					mMountedStorages.push_back(TMountedStorageInfo { pStorage, aliasPath });
				}
			}
		}

		if (pStorage)
		{
			E_RESULT_CODE result = RC_OK;

			if (RC_OK != (result = pStorage->OnMounted(unifiedAliasPath)))
			{
				return result;
			}
		}

		LOG_MESSAGE("[File System] A new virtual path was mounted (" + pStorage->GetBasePath() + " -> " + unifiedAliasPath + ")");

		return RC_OK;
	}

	std::string CBaseFileSystem::_normalizePathView(const std::string& path, bool isDirectory) const
	{
		const std::string pathSeparator { GetPathSeparatorChar() };
		const std::string altPathSeparator { GetAltPathSeparatorChar() };

		// Replace all alternative separators onto main separator
		std::string normalizedPath = Wrench::StringUtils::ReplaceAll(path, altPathSeparator, pathSeparator);

		// Add '/' separator at the end of path if it's a directory
		if (!Wrench::StringUtils::EndsWith(normalizedPath, pathSeparator) && isDirectory)
		{
			normalizedPath.append(pathSeparator);
		}

		// Remove duplicates like the following \\ or //
		normalizedPath = Wrench::StringUtils::ReplaceAll(normalizedPath, pathSeparator + pathSeparator, pathSeparator);
		normalizedPath = Wrench::StringUtils::ReplaceAll(normalizedPath, altPathSeparator + altPathSeparator, altPathSeparator);

		TDE2_ASSERT(IsPathValid(normalizedPath));

		return normalizedPath;
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

	E_RESULT_CODE CBaseFileSystem::RemoveFile(const std::string& filePath)
	{
		if (filePath.empty())
		{
			return RC_INVALID_ARGS;
		}

		return std::experimental::filesystem::remove(filePath) ? RC_OK : RC_FAIL;
	}

	std::vector<std::string> CBaseFileSystem::GetFilesListAtDirectory(const std::string& path) const
	{
		for (auto iter = mMountedStorages.cbegin(); iter != mMountedStorages.cend(); ++iter)
		{
			auto pCurrStorage = iter->mpStorage;

			if (pCurrStorage->PathExists(path))
			{
				return std::move(pCurrStorage->GetFilesListAtDirectory(path));
			}
		}

		return {};
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

	TResult<TFileFactory> CBaseFileSystem::GetFileFactory(TypeId typeId)
	{
		auto&& iter = mFileFactoriesMap.find(typeId);
		if (iter != mFileFactoriesMap.cend())
		{
			return mFileFactories[iter->second];
		}

		return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
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

	std::string CBaseFileSystem::CombinePath(const std::string& left, const std::string& right) const
	{
		std::string resultPath = _normalizePathView(left, true);
		resultPath.append(_normalizePathView(right, false));

		return _normalizePathView(resultPath, false);
	}

	TResult<TFileEntryId> CBaseFileSystem::_openFile(const TypeId& typeId, const std::string& filename, bool createIfDoesntExist)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		const std::string filePath = _normalizePathView(filename, false);

		for (auto&& currMountPointEntry : mMountedStorages)
		{
			const std::string& alias = currMountPointEntry.mAliasPath;

			/// \note Try to open file with current storage, but it could fail
			if (Wrench::StringUtils::StartsWith(filename, alias))
			{
				// \note Firstly, try to open file without resolving path
				if (auto openFileResult = currMountPointEntry.mpStorage->OpenFile(typeId, filePath, createIfDoesntExist))
				{
					return openFileResult;
				}

				// \note If we go here, it means a file doesn't exist at filename path, resolve this path and try again
				if (auto openFileResult = currMountPointEntry.mpStorage->OpenFile(typeId, _resolveVirtualPathInternal(currMountPointEntry, filePath, false), createIfDoesntExist))
				{
					return openFileResult;
				}
			}
		}

		LOG_ERROR(Wrench::StringUtils::Format("[File System] Could not load the specified file from \"{0}\"", filePath));

		return Wrench::TErrValue<E_RESULT_CODE>(RC_FILE_NOT_FOUND);
	}
	
	E_RESULT_CODE CBaseFileSystem::_registerFileFactory(const TypeId& typeId, const TFileFactory& fileFactoryInfo)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (!fileFactoryInfo.mCallback) /// \todo check typeId equals to InvalidTypeId
		{
			return RC_INVALID_ARGS;
		}

		TFileFactoriesRegistry::const_iterator factoryIter = mFileFactoriesMap.find(typeId);

		if (factoryIter != mFileFactoriesMap.cend())
		{
			return RC_FAIL;
		}
		
		mFileFactoriesMap[typeId] = mFileFactories.Add(fileFactoryInfo);
		
		LOG_MESSAGE("[File System] A new file factory was registred by the manager (TypeID : " + ToString<TypeId>(typeId) + ")");

		return RC_OK;
	}

	E_RESULT_CODE CBaseFileSystem::_unregisterFileFactory(const TypeId& typeId)
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

		LOG_MESSAGE("[File System] The folowing file factory was unregistred by the manager (TypeID : " + ToString<TypeId>(typeId) + ")");

		return RC_OK;
	}

	IFile* CBaseFileSystem::_getFile(TFileEntryId fileId)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _getFileUnsafe(fileId);
	}

	IFile* CBaseFileSystem::_getFileUnsafe(TFileEntryId fileId)
	{
		if (fileId == TFileEntryId::Invalid)
		{
			return nullptr;
		}
		
		for (auto&& currMountPoint : mMountedStorages)
		{
			if (auto getFileResult = currMountPoint.mpStorage->GetFile(fileId))
			{
				IFile* pFile = getFileResult.Get();

				/// to prevent deletion of a file by one thread when it can be used in another we need to increment its internal counter's value
				/// later used should Close it. This action will decrement internal counter's value and may destroy the file.
				if (!pFile->IsParentThread())
				{
					pFile->AddRef();
				}

				return pFile;
			}
		}

		return nullptr;
	}

	void CBaseFileSystem::_createNewFile(const std::string& filename)
	{
		std::ofstream newFileInstance(filename.c_str());
		newFileInstance.close();
	}

	std::string CBaseFileSystem::_resolveVirtualPathInternal(const TMountedStorageInfo& mountInfo, const std::string& path, bool isDirectory) const
	{
		const std::string nativeFileSystemPath = mountInfo.mpStorage->GetBasePath();

		if (_normalizePathView(mountInfo.mAliasPath) == path)
		{
			return nativeFileSystemPath;
		}

		return _normalizePathView(nativeFileSystemPath + path.substr(mountInfo.mAliasPath.length()), isDirectory);

#if 0
		// Extract base path from filename
		size_t pos = 0;
		
		std::string filename;
		std::string currPath = path;

		do
		{
			pos = currPath.find_last_of(GetPathSeparatorChar());

			if (pos == std::string::npos)
			{
				if (currPath == path)
				{
					return path;
				}

				return nativeFileSystemPath + path;
			}

			filename = currPath.substr(pos + 1);
			currPath = currPath.substr(0, pos);
		} while (filename.empty() && !currPath.empty());

		return nativeFileSystemPath + filename;
#endif
	}
}