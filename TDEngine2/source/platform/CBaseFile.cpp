#include "./../../include/platform/CBaseFile.h"
#include "./../../include/core/IFileSystem.h"


namespace TDEngine2
{

	CBaseFile::CBaseFile()
	{
	}

	E_RESULT_CODE CBaseFile::Open(IFileSystem* pFileSystem, const std::string& filename)
	{
		if (!pFileSystem)
		{
			return RC_INVALID_ARGS;
		}

		if (mFile.is_open())
		{
			return RC_FAIL;
		}

		mFile.open(filename, std::ios::in);

		if (!mFile.is_open())
		{
			return RC_FILE_NOT_FOUND;
		}

		mName = filename;

		mpFileSystemInstance = pFileSystem;

		return RC_OK;
	}

	E_RESULT_CODE CBaseFile::Close()
	{
		if (!mFile.is_open())
		{
			return RC_FAIL;
		}

		mFile.close();

		E_RESULT_CODE result = mpFileSystemInstance->CloseFile(this);

		if (result != RC_OK)
		{
			return result;
		}

		if ((result = _onFree()) != RC_OK)
		{
			return result;
		}

		delete this;

		return RC_OK;
	}

	std::string CBaseFile::GetFilename() const
	{
		return mName;
	}

	bool CBaseFile::IsOpen() const
	{
		return mFile.is_open();
	}
}