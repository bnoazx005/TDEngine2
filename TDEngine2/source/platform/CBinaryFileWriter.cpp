#include "./../../include/platform/CBinaryFileWriter.h"
#include "./../../include/core/IFileSystem.h"
#include "./../../include/core/IJobManager.h"
#include <functional>


namespace TDEngine2
{
	CBinaryFileWriter::CBinaryFileWriter() :
		CBaseFile()
	{
		mCreationFlags = std::ios::out | std::ios::binary;
	}

	E_RESULT_CODE CBinaryFileWriter::Write(const void* pBuffer, U32 bufferSize)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (!mFile.is_open())
		{
			return RC_FAIL;
		}

		mFile.write(reinterpret_cast<const C8*>(pBuffer), bufferSize);
		
		if (mFile.bad())
		{
			return RC_FAIL;
		}

		return RC_OK;
	}

	void CBinaryFileWriter::WriteAsync(const void* pBuffer, U32 bufferSize, const TSuccessWriteCallback& successCallback,
		const TErrorWriteCallback& errorCallback)
	{
		if (!mpFileSystemInstance->IsStreamingEnabled())
		{
			errorCallback(RC_ASYNC_FILE_IO_IS_DISABLED);

			return;
		}

		IJobManager* pJobManager = mpFileSystemInstance->GetJobManager();

		/// \note Is it safe to work with pBuffer without lock
		pJobManager->SubmitJob(std::function([successCallback, errorCallback, pBuffer](CBinaryFileWriter* pFileWriter, U32 size)
		{
			E_RESULT_CODE result = RC_OK;

			if ((result = pFileWriter->Write(pBuffer, size)) != RC_OK)
			{
				errorCallback(result);
			}

			successCallback();

		}), this, bufferSize);
	}

	E_RESULT_CODE CBinaryFileWriter::Flush()
	{
		std::lock_guard<std::mutex> lock(mMutex);

		mFile.flush();

		if (mFile.bad())
		{
			return RC_FAIL;
		}

		return RC_OK;
	}

	E_RESULT_CODE CBinaryFileWriter::SetPosition(U32 pos)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		mFile.seekg(pos);

		return RC_OK;
	}

	bool CBinaryFileWriter::IsEOF() const
	{
		std::lock_guard<std::mutex> lock(mMutex);

		return mFile.eof();
	}

	U32 CBinaryFileWriter::GetPosition() const
	{
		std::lock_guard<std::mutex> lock(mMutex);

		return static_cast<U32>(mFile.tellg());
	}

	E_RESULT_CODE CBinaryFileWriter::_onFree()
	{
		return RC_OK;
	}


	IFile* CreateBinaryFileWriter(IFileSystem* pFileSystem, const std::string& filename, E_RESULT_CODE& result)
	{
		CBinaryFileWriter* pFileInstance = new (std::nothrow) CBinaryFileWriter();

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