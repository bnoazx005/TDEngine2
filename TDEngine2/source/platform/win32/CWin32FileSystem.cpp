#include "./../../../include/platform/win32/CWin32FileSystem.h"
#include "./../../../include/core/IFile.h"
#include "./../../../include/platform/CTextFileReader.h"
#include <algorithm>
#if _HAS_CXX17
#include <filesystem>
#else
#include <fstream>
#endif


namespace TDEngine2
{
	C8 CWin32FileSystem::mPathSeparator    = '\\';

	C8 CWin32FileSystem::mAltPathSeparator = '/';

	std::string CWin32FileSystem::mInvalidPath = "";

	C8 CWin32FileSystem::mForbiddenChars[] { '<', '>', '*', '?',  '|', '\"', ':', '\0' };


	CWin32FileSystem::CWin32FileSystem():
		CBaseObject()
	{
	}

	E_RESULT_CODE CWin32FileSystem::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}
		
		mIsInitialized = true;

		/// mount a root directory
		Mount(this->GetCurrDirectory(), std::string(1, mPathSeparator));

		return RC_OK;
	}

	E_RESULT_CODE CWin32FileSystem::Free()
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

		return RC_OK;
	}

	E_RESULT_CODE CWin32FileSystem::Mount(const std::string& path, const std::string& aliasPath)
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		std::string unifiedPath      = _unifyPathView(path);
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

		return RC_OK;
	}

	E_RESULT_CODE CWin32FileSystem::Unmount(const std::string& aliasPath)
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

		return RC_OK;
	}

	std::string CWin32FileSystem::ResolveVirtualPath(const std::string& path) const
	{
		std::string unifiedPath = _unifyPathView(path);

		if (path.empty() || (path.size() > 1 && path[0] != mPathSeparator))
		{
			return path; // this case includes all real paths, single files
		}

		if (path.length() == 1 && path[0] == mPathSeparator) // replace / virtual root directory with current directory of an application
		{
			return this->GetCurrDirectory();
		}
		
		U32 prevDirectoryPos = 0;
		U32 currDirectoryPos = 0;
		U32 unchechedPathPos = 0;

		std::string currPath, lastMatchPart;

		TVirtualPathsMap::const_iterator physicalPathIter;

		/// select the greatest coincidental part of the path
		while ((currDirectoryPos = unifiedPath.find_first_of(mPathSeparator, prevDirectoryPos)) != std::string::npos)
		{
			prevDirectoryPos = currDirectoryPos + 1;

			currPath = unifiedPath.substr(0, prevDirectoryPos);

			physicalPathIter = mVirtualPathsMap.find(currPath);

			if (physicalPathIter != mVirtualPathsMap.cend())
			{
				lastMatchPart = (*physicalPathIter).second;

				unchechedPathPos = currDirectoryPos;
			}
		}

		// check special case when last part of a path doesn't contains an extension's declaration like so \\foo
		// in this situation we should try to interpretate it as a possible virtual path
		if ((currPath = unifiedPath.substr(prevDirectoryPos - 1) + mPathSeparator).find('.') == std::string::npos)
		{
			// the part could be a directory and a filename as well

			physicalPathIter = mVirtualPathsMap.find(currPath);

			if (physicalPathIter != mVirtualPathsMap.cend())
			{
				return (*physicalPathIter).second;
			}
		}

		return lastMatchPart + unifiedPath.substr(unchechedPathPos, unifiedPath.length() - unchechedPathPos);
	}

	ITextFileReader* CWin32FileSystem::CreateTextFileReader(const std::string& filename, E_RESULT_CODE& result)
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

		IFileReader* pTextFileReader = TDEngine2::CreateTextFileReader(this, filename, result);

		if (result != RC_OK)
		{
			return nullptr;
		}

		result = _registerFileEntry(pTextFileReader);

		if (result != RC_OK)
		{
			return nullptr;
		}

		return dynamic_cast<ITextFileReader*>(pTextFileReader);
	}

	E_RESULT_CODE CWin32FileSystem::CloseFile(IFile* pFile)
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

		return RC_OK;
	}

	E_RESULT_CODE CWin32FileSystem::CloseAllFiles()
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

	bool CWin32FileSystem::FileExists(const std::string& filename) const
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

	std::string CWin32FileSystem::GetCurrDirectory() const
	{
#if _HAS_CXX17
		return std::experimental::filesystem::current_path().string();
#else
		return ".";
#endif
	}

	E_ENGINE_SUBSYSTEM_TYPE CWin32FileSystem::GetType() const
	{
		return EST_FILE_SYSTEM;
	}

	E_RESULT_CODE CWin32FileSystem::_registerFileEntry(IFile* pFileEntry)
	{
		U32 hashValue = mActiveFiles.size();

		mActiveFiles.push_back(pFileEntry);

		mFilesMap[pFileEntry->GetFilename()] = hashValue;

		return RC_OK;
	}

	std::string CWin32FileSystem::_unifyPathView(const std::string& path, bool isVirtualPath) const
	{
		if (path.empty())
		{
			return mInvalidPath;
		}

		std::string unifiedPath = path;

		std::replace(unifiedPath.begin(), unifiedPath.end(), mAltPathSeparator, mPathSeparator);

		// check up is the path valid
		if (!_isPathValid(unifiedPath, isVirtualPath))
		{
			return mInvalidPath;
		}

		if (isVirtualPath && unifiedPath[unifiedPath.length() - 1] != mPathSeparator)
		{
			return unifiedPath + mPathSeparator;
		}

		return unifiedPath;
	}

	bool CWin32FileSystem::_isPathValid(const std::string& path, bool isVirtualPath) const
	{
		bool isPathEmpty                = path.empty();
		bool pathContainsForbiddenChars = (path.find(mForbiddenChars, 0, sizeof(mForbiddenChars) / sizeof(mForbiddenChars[0])) != std::string::npos) && (path.find(':') != 1);
		bool virtualPathContainsDots    = isVirtualPath && path.find('.') != std::string::npos;
		
		return !isPathEmpty && !pathContainsForbiddenChars && !virtualPathContainsDots;
	}


	TDE2_API IFileSystem* CreateWin32FileSystem(E_RESULT_CODE& result)
	{
		CWin32FileSystem* pFileSystemInstance = new (std::nothrow) CWin32FileSystem();

		if (!pFileSystemInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pFileSystemInstance->Init();

		if (result != RC_OK)
		{
			delete pFileSystemInstance;

			pFileSystemInstance = nullptr;
		}

		return dynamic_cast<IFileSystem*>(pFileSystemInstance);
	}
}