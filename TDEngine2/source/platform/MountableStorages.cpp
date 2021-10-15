#include "../../include/platform/MountableStorages.h"
#include "../../include/platform/IOStreams.h"
#include "../../include/core/CBaseFileSystem.h"
#include "../../include/core/IFile.h"
#include "../../include/utils/CFileLogger.h"
#include "stringUtils.hpp"
#include <fstream>

#if _HAS_CXX17
	#include <filesystem>
	namespace fs = std::filesystem;
#else
	#include <experimental/filesystem>
	namespace fs = std::experimental::filesystem;
#endif


namespace TDEngine2
{
	/*!
		\brief CBaseMountableStorage's definition
	*/

	CBaseMountableStorage::CBaseMountableStorage():
		CBaseObject(), mpFileSystem(nullptr)
	{
	}

	E_RESULT_CODE CBaseMountableStorage::Init(IFileSystem* pFileSystem, const std::string& path)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pFileSystem || path.empty())
		{
			return RC_INVALID_ARGS;
		}

		mpFileSystem = pFileSystem;
		mPhysicalPath = path;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CBaseMountableStorage::_onFreeInternal()
	{
		E_RESULT_CODE result = RC_OK;

		for (U32 i = 0; i < mActiveFiles.GetSize(); ++i)
		{
			if (auto currFile = mActiveFiles[i])
			{
				if (IFile* pFile = currFile.Get())
				{
					result = result | pFile->Close();
				}
			}
		}

		return result;
	}

	TResult<TFileEntryId> CBaseMountableStorage::OpenFile(const TypeId& typeId, const std::string& path, bool createIfDoesntExist)
	{
		if (!mpFileSystem->IsPathValid(path))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		auto getFileFactoryResult = mpFileSystem->GetFileFactory(typeId);
		if (getFileFactoryResult.HasError())
		{
			return Wrench::TErrValue<E_RESULT_CODE>(getFileFactoryResult.GetError());
		}

		const TFileFactory& fileFactoryDesc = getFileFactoryResult.Get();

		/// create a file's entry on a disk if doesn't exist but a user asks for it
		if (!FileExists(path) && createIfDoesntExist)
		{
			LOG_MESSAGE("[File System] A new file was created (TypeID : " + ToString<TypeId>(typeId) + "; path: " + path + ")");
			_createNewFile(path);
		}

		E_RESULT_CODE result = RC_OK;

		auto createStreamResult = _createStream(path, fileFactoryDesc.mFileType);
		if (createStreamResult.HasError())
		{
			return Wrench::TErrValue<E_RESULT_CODE>(createStreamResult.GetError());
		}

		IFile* pNewFileInstance = fileFactoryDesc.mCallback(this, createStreamResult.Get(), result);
		if (result != RC_OK)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		TFileEntryId newFileEntryId = _registerFileEntry(pNewFileInstance);

		LOG_MESSAGE("[File System] A new file descriptor was created by the manager (" + path + ")");

		return Wrench::TOkValue<TFileEntryId>(newFileEntryId);
	}

	E_RESULT_CODE CBaseMountableStorage::CloseFile(TFileEntryId handle)
	{
		auto iter = mLocalConversionMap.find(handle);
		if (iter == mLocalConversionMap.cend())
		{
			return RC_FILE_NOT_FOUND;
		}

		mActiveFiles.RemoveAt(iter->second);
		mLocalConversionMap.erase(handle);

		for (auto& currEntry : mFilesMap)
		{
			if (currEntry.second == handle)
			{
				LOG_MESSAGE("[File System] Existing file descriptor was closed (" + currEntry.first + ")");

				mFilesMap.erase(currEntry.first);
				break;
			}
		}

		return RC_OK;
	}

	E_RESULT_CODE CBaseMountableStorage::CloseFile(const std::string& path)
	{
		auto iter = mFilesMap.find(path);
		if (iter == mFilesMap.cend())
		{
			return RC_FILE_NOT_FOUND;
		}

		return CloseFile(iter->second);
	}

	TResult<IFile*> CBaseMountableStorage::GetFile(TFileEntryId fileId) const
	{
		auto iter = mLocalConversionMap.find(fileId);

		if ((fileId == TFileEntryId::Invalid) || (iter == mLocalConversionMap.cend()))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FILE_NOT_FOUND);
		}

		return mActiveFiles[iter->second];
	}

	IFileSystem* CBaseMountableStorage::GetFileSystem() const
	{
		return mpFileSystem;
	}

	const std::string& CBaseMountableStorage::GetBasePath() const
	{
		return mPhysicalPath;
	}

	TFileEntryId CBaseMountableStorage::_registerFileEntry(IFile* pFileEntry)
	{
		USIZE localHandle = mActiveFiles.Add(pFileEntry);

		const std::string& path = pFileEntry->GetFilename();

		TFileEntryId handle = static_cast<TFileEntryId>(TDE2_STRING_ID(path.c_str()));

		mFilesMap[path] = handle;
		mLocalConversionMap[handle] = localHandle;

		return handle;
	}


	/*!
		\brief CPhysicalFilesStorage's definition
	*/

	CPhysicalFilesStorage::CPhysicalFilesStorage() :
		CBaseMountableStorage()
	{
	}
		
	E_RESULT_CODE CPhysicalFilesStorage::OnMounted(const std::string&)
	{
		return RC_OK;
	}

	bool CPhysicalFilesStorage::FileExists(const std::string& path) const
	{
#if _HAS_CXX17
		return std::experimental::filesystem::exists(path);
#else
		std::ifstream file(path);

		bool exists = file.good();

		file.close();

		return exists;
#endif
	}

	U16 CPhysicalFilesStorage::GetPriority() const
	{
		return static_cast<U16>(E_MOUNTABLE_STORAGES_PRIORITIES::NATIVE);
	}

	std::vector<std::string> CPhysicalFilesStorage::GetFilesListAtDirectory(const std::string& path) const
	{
		std::vector<std::string> filesList;

		for (const auto & entry : std::experimental::filesystem::directory_iterator(path))
		{
			filesList.emplace_back(entry.path().string());
		}
		
		return std::move(filesList);
	}

	bool CPhysicalFilesStorage::PathExists(const std::string& path) const
	{
		return std::experimental::filesystem::exists(path);
	}

	void CPhysicalFilesStorage::_createNewFile(const std::string& path)
	{
		std::ofstream newFileInstance(path.c_str());
		newFileInstance.close();
	}

	TResult<IStream*> CPhysicalFilesStorage::_createStream(const std::string& path, E_FILE_FACTORY_TYPE type) const
	{
		E_RESULT_CODE result = RC_OK;
		
		IStream* pStream = (type == E_FILE_FACTORY_TYPE::READER) ? CreateFileInputStream(path, result) : CreateFileOutputStream(path, result);

		if (result != RC_OK || !pStream)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		return Wrench::TOkValue<IStream*>(pStream);
	}

	IMountableStorage* CreatePhysicalFilesStorage(IFileSystem* pFileSystem, const std::string& path, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IMountableStorage, CPhysicalFilesStorage, result, pFileSystem, path);
	}


	/*!
		\brief CPackageFilesStorage's definition
	*/

	CPackageFilesStorage::CPackageFilesStorage() :
		CBaseMountableStorage()
	{
	}

	E_RESULT_CODE CPackageFilesStorage::OnMounted(const std::string& aliasPath)
	{
		mAliasPath = aliasPath;

		auto openPackageResult = mpFileSystem->Open<IPackageFileReader>(mPhysicalPath);

		if (openPackageResult.IsOk())
		{
			IPackageFileReader* pPackageFileReader = mpFileSystem->Get<IPackageFileReader>(mPackageFileHandle = openPackageResult.Get());

			const auto& filesTable = pPackageFileReader->GetFilesTable();

			for (auto&& currFileEntry : filesTable)
			{
				mPackageFilesInfoTable.insert({ currFileEntry.mFilename, { currFileEntry.mDataBlockOffset, currFileEntry.mDataBlockSize } });
			}

			return RC_OK;
		}

		return openPackageResult.GetError();
	}

	bool CPackageFilesStorage::FileExists(const std::string& path) const
	{
		return (mPackageFilesInfoTable.find(path) != mPackageFilesInfoTable.cend());
	}

	U16 CPackageFilesStorage::GetPriority() const
	{
		return static_cast<U16>(E_MOUNTABLE_STORAGES_PRIORITIES::PACKAGE);
	}

	std::vector<std::string> CPackageFilesStorage::GetFilesListAtDirectory(const std::string& path) const
	{
		TDE2_UNIMPLEMENTED();
		return {};
	}

	bool CPackageFilesStorage::PathExists(const std::string& path) const
	{
		auto it = std::find_if(mPackageFilesInfoTable.cbegin(), mPackageFilesInfoTable.cend(), [path](auto&& entry)
		{
			return entry.first.find(path) != std::string::npos;
		});

		return it != mPackageFilesInfoTable.cend();
	}

	void CPackageFilesStorage::_createNewFile(const std::string& path)
	{
		LOG_ERROR("[Package File Storage] \"Create a new file\" feature doesn't supported for now");
		TDE2_UNIMPLEMENTED();
	}

	TResult<IStream*> CPackageFilesStorage::_createStream(const std::string& path, E_FILE_FACTORY_TYPE type) const
	{
		E_RESULT_CODE result = RC_OK;

		IPackageFileReader* pPackageFile = mpFileSystem->GetUnsafe<IPackageFileReader>(mPackageFileHandle);
		if (!pPackageFile)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		auto pStream = CreateMemoryIOStream(path, pPackageFile->ReadFileBytes(Wrench::StringUtils::ReplaceAll(path, mAliasPath, Wrench::StringUtils::GetEmptyStr())), result);

		if (result != RC_OK || !pStream)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		return Wrench::TOkValue<IStream*>(pStream);
	}


	IMountableStorage* CreatePackageFilesStorage(IFileSystem* pFileSystem, const std::string& path, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IMountableStorage, CPackageFilesStorage, result, pFileSystem, path);
	}
}