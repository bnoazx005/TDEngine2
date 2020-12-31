#include "./../../../include/platform/win32/CWin32FileSystem.h"
#include "./../../../include/core/IFile.h"
#include "./../../../include/platform/CTextFileReader.h"
#include "stringUtils.hpp"
#include <algorithm>

#if defined(TDE2_USE_WIN32PLATFORM)
#include <ShlObj_core.h>
#endif

#if _HAS_CXX17
#include <filesystem>
#else
#include <fstream>
#endif


namespace TDEngine2
{
	C8 CWin32FileSystem::mPathSeparator = '\\';

	C8 CWin32FileSystem::mAltPathSeparator = '/';
	
	C8 CWin32FileSystem::mForbiddenChars[]{ '<', '>', '*', '?',  '|', '\"', ':', '\0' };

	CWin32FileSystem::CWin32FileSystem() :
		CBaseFileSystem()
	{
	}

	std::string CWin32FileSystem::GetApplicationDataPath() const
	{
		C8 appDataDirectoryPath[MAX_PATH];

#if defined(TDE2_USE_WIN32PLATFORM)
		if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, appDataDirectoryPath)))
		{
			return std::string(appDataDirectoryPath);
		}
#endif

		return GetCurrDirectory();
	}

	std::string CWin32FileSystem::GetUserDirectory() const
	{
		C8 userDirectoryPath[MAX_PATH];

#if defined(TDE2_USE_WIN32PLATFORM)
		if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PERSONAL | CSIDL_FLAG_CREATE, NULL, 0, userDirectoryPath)))
		{
			return std::string(userDirectoryPath);
		}
#endif

		return GetCurrDirectory();
	}

	bool CWin32FileSystem::IsPathValid(const std::string& path, bool isVirtualPath) const
	{
		bool isPathEmpty                = path.empty();
		bool pathContainsForbiddenChars = (path.find(mForbiddenChars, 0, sizeof(mForbiddenChars) / sizeof(mForbiddenChars[0])) != std::string::npos) && (path.find(':') != 1);
		bool virtualPathContainsDots    = isVirtualPath && path.find("..") != std::string::npos;
		
		return !isPathEmpty && !pathContainsForbiddenChars && !virtualPathContainsDots;
	}

	E_RESULT_CODE CWin32FileSystem::_onInit()
	{
		/// mount a root directory
		return MountPhysicalPath(this->GetCurrDirectory(), Wrench::StringUtils::GetEmptyStr());
	}

	const C8& CWin32FileSystem::GetPathSeparatorChar() const
	{
		return mPathSeparator;
	}

	const C8& CWin32FileSystem::GetAltPathSeparatorChar() const
	{
		return mAltPathSeparator;
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