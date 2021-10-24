#include "../../include/platform/CBaseFile.h"
#include "../../include/platform/MountableStorages.h"
#include "../../include/platform/IOStreams.h"
#include "../../include/core/IFileSystem.h"


namespace TDEngine2
{

	CBaseFile::CBaseFile():
		mRefCounter(1)
	{
	}

	E_RESULT_CODE CBaseFile::Open(IMountableStorage* pStorage, TPtr<IStream> pStream)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (!pStorage || !pStream)
		{
			return RC_INVALID_ARGS;
		}

		mpStreamImpl = pStream;

		if (!mpStreamImpl->IsValid())
		{
			return RC_FILE_NOT_FOUND;
		}

		E_RESULT_CODE result = RC_OK;

		if ((result = _onInit()) != RC_OK)
		{
			return result;
		}

		mRefCounter = 1;

		mName = mpStreamImpl->GetName();

		mpStorage = pStorage;
		
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

			mpStreamImpl = nullptr;

			if ((result = mpStorage->CloseFile(mName)) != RC_OK)
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

	std::string CBaseFile::GetShortName() const
	{
		const IFileSystem* pFileSystem = mpStorage->GetFileSystem();

		size_t pos = mName.find_last_of(pFileSystem->GetPathSeparatorChar());

		if ((pos != std::string::npos) || (pos = mName.find_last_of(pFileSystem->GetAltPathSeparatorChar())) != std::string::npos)
		{
			return mName.substr(pos + 1);
		}

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

	TPtr<IStream> CBaseFile::GetStream() const
	{
		return mpStreamImpl;
	}

	E_RESULT_CODE CBaseFile::_onInit()
	{
		return RC_OK;
	}
}