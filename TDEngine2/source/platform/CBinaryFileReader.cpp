#include "./../../include/platform/CBinaryFileReader.h"
#include "./../../include/core/IFileSystem.h"
#include "./../../include/core/IJobManager.h"
#include <functional>
#include <limits>


namespace TDEngine2
{
	CBinaryFileReader::CBinaryFileReader():
		CBaseFile()
	{
		mCreationFlags = std::ios::in | std::ios::binary;
	}

	E_RESULT_CODE CBinaryFileReader::Read(void* pBuffer, U32 bufferSize)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (!pBuffer)
		{
			return RC_INVALID_ARGS;
		}

		mFile.read(static_cast<C8*>(pBuffer), bufferSize);

		if (mFile.bad() || mFile.fail())
		{
			return RC_FAIL;
		}

		return RC_OK;
	}

	void CBinaryFileReader::ReadAsync(U32 size, const TSuccessReadCallback& successCallback, const TErrorReadCallback& errorCallback)
	{
		if (!mpFileSystemInstance->IsStreamingEnabled())
		{
			errorCallback(RC_ASYNC_FILE_IO_IS_DISABLED);

			return;
		}

		IJobManager* pJobManager = mpFileSystemInstance->GetJobManager();
		
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

		mFile.seekg(pos);

		return RC_OK;
	}

	bool CBinaryFileReader::IsEOF() const
	{
		std::lock_guard<std::mutex> lock(mMutex);

		return mFile.eof();
	}

	U32 CBinaryFileReader::GetPosition() const
	{
		std::lock_guard<std::mutex> lock(mMutex);

		return static_cast<U32>(mFile.tellg());
	}

	E_RESULT_CODE CBinaryFileReader::_onFree()
	{
		return RC_OK;
	}

	U64 CBinaryFileReader::GetFileLength() const
	{
		if (!mFile.is_open())
		{
			return 0;
		}

		mFile.ignore((std::numeric_limits<std::streamsize>::max)());

		std::streamsize length = mFile.gcount();

		mFile.clear();   // \note Since ignore will have set eof.
		mFile.seekg(0, std::ios_base::beg);

		return static_cast<U64>(length);
	}


	IFile* CreateBinaryFileReader(IFileSystem* pFileSystem, const std::string& filename, E_RESULT_CODE& result)
	{
		CBinaryFileReader* pFileInstance = new (std::nothrow) CBinaryFileReader();

		if (!pFileInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pFileInstance->Open(pFileSystem, filename);

		if (result != RC_OK)
		{
			delete pFileInstance;

			pFileInstance = nullptr;
		}

		return dynamic_cast<IFile*>(pFileInstance);
	}
}