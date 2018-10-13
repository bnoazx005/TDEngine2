#include "./../../../include/platform/unix/CUnixFileSystem.h"
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
	C8 CUnixFileSystem::mPathSeparator = '/';

	C8 CUnixFileSystem::mAltPathSeparator = '\\';
	
	C8 CUnixFileSystem::mForbiddenChars[]{ '<', '>', '*', '?',  '|', '\"', ':', '\0' };

	CUnixFileSystem::CUnixFileSystem():
		CBaseFileSystem()
	{
	}
	
	std::string CUnixFileSystem::ResolveVirtualPath(const std::string& path) const
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

	std::string CUnixFileSystem::_unifyPathView(const std::string& path, bool isVirtualPath) const
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

	bool CUnixFileSystem::_isPathValid(const std::string& path, bool isVirtualPath) const
	{
		bool isPathEmpty = path.empty();
		bool pathContainsForbiddenChars = (path.find(mForbiddenChars, 0, sizeof(mForbiddenChars) / sizeof(mForbiddenChars[0])) != std::string::npos);
		bool virtualPathContainsDots = isVirtualPath && path.find('.') != std::string::npos;

		return !isPathEmpty && !pathContainsForbiddenChars && !virtualPathContainsDots;
	}

	E_RESULT_CODE CUnixFileSystem::_onInit()
	{
		/// mount a root directory
		return Mount(this->GetCurrDirectory(), std::string(1, mPathSeparator));
	}


	TDE2_API IFileSystem* CreateUnixFileSystem(E_RESULT_CODE& result)
	{
		CUnixFileSystem* pFileSystemInstance = new (std::nothrow) CUnixFileSystem();

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