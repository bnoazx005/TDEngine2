#include "./../../include/platform/CBaseFile.h"
#include "./../../include/core/IFileSystem.h"
#include "../../include/platform/IOStreams.h"


namespace TDEngine2
{

	CBaseFile::CBaseFile():
		mRefCounter(1)
	{
	}

	E_RESULT_CODE CBaseFile::Open(IFileSystem* pFileSystem, IStream* pStream)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (!pFileSystem || !pStream)
		{
			return RC_INVALID_ARGS;
		}

		mpStreamImpl = pStream;

		if (!mpStreamImpl->IsValid())
		{
			return RC_FILE_NOT_FOUND;
		}

		mRefCounter = 1;

		mName = mpStreamImpl->GetName();

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
			
			E_RESULT_CODE result = RC_OK;

			if ((result = _onFree()) != RC_OK)
			{
				return result;
			}

			if (mpStreamImpl)
			{
				result = result | mpStreamImpl->Free();
			}

			if ((result = mpFileSystemInstance->CloseFile(mName)) != RC_OK)
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

		return mpStreamImpl->IsValid();
	}

	bool  CBaseFile::IsParentThread() const
	{
		return mCreatorThreadId == std::this_thread::get_id();
	}
}