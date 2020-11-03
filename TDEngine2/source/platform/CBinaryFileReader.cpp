#include "../../include/platform/CBinaryFileReader.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/core/IJobManager.h"
#include "../../include/platform/IOStreams.h"
#include "../../include/platform/MountableStorages.h"
#include <functional>
#include <limits>


namespace TDEngine2
{
	CBinaryFileReader::CBinaryFileReader():
		CBaseFile(), mpCachedInputStream(nullptr)
	{
	}

	E_RESULT_CODE CBinaryFileReader::Read(void* pBuffer, U32 bufferSize)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (!pBuffer)
		{
			return RC_INVALID_ARGS;
		}

		_getInputStream()->Read(pBuffer, bufferSize);

		if (!mpStreamImpl->IsValid())
		{
			return RC_FAIL;
		}

		return RC_OK;
	}

	void CBinaryFileReader::ReadAsync(U32 size, const TSuccessReadCallback& successCallback, const TErrorReadCallback& errorCallback)
	{
		IFileSystem* pFileSystem = mpStorage->GetFileSystem();

		if (!pFileSystem->IsStreamingEnabled())
		{
			errorCallback(RC_ASYNC_FILE_IO_IS_DISABLED);

			return;
		}

		IJobManager* pJobManager = pFileSystem->GetJobManager();
		
		pJobManager->SubmitJob(std::function<void(CBinaryFileReader*, U32)>([successCallback, errorCallback](CBinaryFileReader* pFileReader, U32 size)
		{
			E_RESULT_CODE result = RC_OK;

			C8* pBuffer = new C8[size];

			if ((result = pFileReader->Read(pBuffer, size)) != RC_OK)
			{
				errorCallback(result);

				delete[] pBuffer;

				return;
			}

			successCallback(pBuffer);

			delete[] pBuffer;
			
		}), this, size);
	}

	E_RESULT_CODE CBinaryFileReader::SetPosition(U32 pos)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return mpStreamImpl->SetPosition(pos);
	}

	bool CBinaryFileReader::IsEOF() const
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return mpStreamImpl->IsEndOfStream();
	}

	U32 CBinaryFileReader::GetPosition() const
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return mpStreamImpl->GetPosition();
	}

	E_RESULT_CODE CBinaryFileReader::_onFree()
	{
		return RC_OK;
	}

	U64 CBinaryFileReader::GetFileLength() const
	{
		if (!mpStreamImpl->IsValid())
		{
			return 0;
		}

		return mpStreamImpl->GetLength();
	}

	IInputStream* CBinaryFileReader::_getInputStream()
	{
		if (!mpCachedInputStream)
		{
			mpCachedInputStream = dynamic_cast<IInputStream*>(mpStreamImpl);
		}

		return mpCachedInputStream;
	}


	IFile* CreateBinaryFileReader(IMountableStorage* pStorage, IStream* pStream, E_RESULT_CODE& result)
	{
		CBinaryFileReader* pFileInstance = new (std::nothrow) CBinaryFileReader();

		if (!pFileInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pFileInstance->Open(pStorage, pStream);

		if (result != RC_OK)
		{
			delete pFileInstance;

			pFileInstance = nullptr;
		}

		return dynamic_cast<IFile*>(pFileInstance);
	}
}