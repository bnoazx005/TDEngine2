#include "./../../include/platform/CBaseFile.h"
#include "./../../include/core/IFileSystem.h"


namespace TDEngine2
{

	CBaseFile::CBaseFile():
		mCreationFlags(std::ios::in|std::ios::out), mRefCounter(1)
	{
	}

	E_RESULT_CODE CBaseFile::Open(IFileSystem* pFileSystem, const std::string& filename)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (!pFileSystem)
		{
			return RC_INVALID_ARGS;
		}

		if (mFile.is_open())
		{
			return RC_FAIL;
		}

		mFile.open(filename, mCreationFlags);

		if (!mFile.is_open())
		{
			return RC_FILE_NOT_FOUND;
		}

		mRefCounter = 1;

		mName = filename;

		mpFileSystemInstance = pFileSystem;
		
		mCreatorThreadId = std::this_thread::get_id();

		return RC_OK;
	}

	void CBaseFile::AddRef()
	{
		++mRefCounter;
	}

	E_RESULT_CODE CBaseFile::Close()
	{
		{
			std::lock_guard<std::mutex> lock(mMutex);

			if (!mFile.is_open())
			{
				return RC_FAIL;
			}

			mFile.close();

			E_RESULT_CODE result = mpFileSystemInstance->CloseFile(mName);

			if (result != RC_OK)
			{
				return result;
			}

			if ((result = _onFree()) != RC_OK)
			{
				return result;
			}
		}

		--mRefCounter;

		if (!mRefCounter)
		{
			delete this;
		}

		return RC_OK;
	}

	std::string CBaseFile::GetFilename() const
	{
		return mName;
	}

	bool CBaseFile::IsOpen() const
	{
		std::lock_guard<std::mutex> lock(mMutex);

		return mFile.is_open();
	}

	bool  CBaseFile::IsParentThread() const
	{
		return mCreatorThreadId == std::this_thread::get_id();
	}
}