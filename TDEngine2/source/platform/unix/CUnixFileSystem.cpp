#include "./../../../include/platform/unix/CUnixFileSystem.h"
#include "./../../../include/core/IFile.h"
#include "./../../../include/platform/CTextFileReader.h"
#include "stringUtils.hpp"
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

	std::string CUnixFileSystem::GetApplicationDataPath() const
	{
		TDE2_UNIMPLEMENTED();
		return Wrench::StringUtils::GetEmptyStr();
	}

	std::string CUnixFileSystem::GetUserDirectory() const
	{
		TDE2_UNIMPLEMENTED();
		return Wrench::StringUtils::GetEmptyStr();
	}

	
	bool CUnixFileSystem::IsPathValid(const std::string& path, bool isVirtualPath) const
	{
		bool isPathEmpty = path.empty();
		bool pathContainsForbiddenChars = (path.find(mForbiddenChars, 0, sizeof(mForbiddenChars) / sizeof(mForbiddenChars[0])) != std::string::npos);
		bool virtualPathContainsDots = isVirtualPath && path.find("..") != std::string::npos;

		return !isPathEmpty && !pathContainsForbiddenChars && !virtualPathContainsDots;
	}

	E_RESULT_CODE CUnixFileSystem::_onInit()
	{
		/// mount a root directory
		return MountPhysicalPath(this->GetCurrDirectory(), Wrench::StringUtils::GetEmptyStr());
	}

	const C8& CUnixFileSystem::GetPathSeparatorChar() const
	{
		return mPathSeparator;
	}

	const C8& CUnixFileSystem::GetAltPathSeparatorChar() const
	{
		return mAltPathSeparator;
	}


	TDE2_API IFileSystem* CreateUnixFileSystem(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IFileSystem, CUnixFileSystem, result);
	}
}