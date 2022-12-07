#include "../../include/platform/CBinaryFileWriter.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/core/IJobManager.h"
#include "../../include/platform/IOStreams.h"
#include "../../include/platform/MountableStorages.h"
#include <functional>


namespace TDEngine2
{
	CBinaryFileWriter::CBinaryFileWriter() :
		CBaseFile(), mpCachedOutputStream(nullptr)
	{
	}

	E_RESULT_CODE CBinaryFileWriter::WriteUInt8(U8 value)
	{
		return Write(&value, sizeof(value));
	}

	E_RESULT_CODE CBinaryFileWriter::WriteUInt16(U16 value)
	{
		value = SwapBytes(value);
		return Write(&value, sizeof(value));
	}

	E_RESULT_CODE CBinaryFileWriter::WriteUInt32(U32 value)
	{
		value = SwapBytes(value);
		return Write(&value, sizeof(value));
	}

	E_RESULT_CODE CBinaryFileWriter::WriteUInt64(U64 value)
	{
		value = SwapBytes(value);
		return Write(&value, sizeof(value));
	}

	E_RESULT_CODE CBinaryFileWriter::WriteInt8(I8 value)
	{
		return Write(&value, sizeof(value));
	}

	E_RESULT_CODE CBinaryFileWriter::WriteInt16(I16 value)
	{
		value = SwapBytes(value);
		return Write(&value, sizeof(value));
	}

	E_RESULT_CODE CBinaryFileWriter::WriteInt32(I32 value)
	{
		value = SwapBytes(value);
		return Write(&value, sizeof(value));
	}

	E_RESULT_CODE CBinaryFileWriter::WriteInt64(I64 value)
	{
		value = SwapBytes(value);
		return Write(&value, sizeof(value));
	}

	E_RESULT_CODE CBinaryFileWriter::WriteFloat(F32 value)
	{
		value = SwapBytes(value);
		return Write(&value, sizeof(value));
	}

	E_RESULT_CODE CBinaryFileWriter::WriteDouble(F64 value)
	{
		value = SwapBytes(value);
		return Write(&value, sizeof(value));
	}

	E_RESULT_CODE CBinaryFileWriter::WriteBool(bool value)
	{
		value = SwapBytes(value);
		return Write(&value, sizeof(value));
	}

	E_RESULT_CODE CBinaryFileWriter::Write(const void* pBuffer, TSizeType bufferSize)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (!mpStreamImpl->IsValid())
		{
			return RC_FAIL;
		}

		_getOutputStream()->Write(reinterpret_cast<const C8*>(pBuffer), bufferSize);
		
		if (!mpStreamImpl->IsValid())
		{
			return RC_FAIL;
		}

		return RC_OK;
	}

	void CBinaryFileWriter::WriteAsync(const void* pBuffer, TSizeType bufferSize, const TSuccessWriteCallback& successCallback,
		const TErrorWriteCallback& errorCallback)
	{
		IFileSystem* pFileSystem = mpStorage->GetFileSystem();

		if (!pFileSystem->IsStreamingEnabled())
		{
			errorCallback(RC_ASYNC_FILE_IO_IS_DISABLED);

			return;
		}

		IJobManager* pJobManager = pFileSystem->GetJobManager();

		/// \note Is it safe to work with pBuffer without lock
		pJobManager->SubmitJob(nullptr, [this, pBuffer, bufferSize, successCallback, errorCallback](auto&&)
		{
			E_RESULT_CODE result = RC_OK;

			if ((result = Write(pBuffer, bufferSize)) != RC_OK)
			{
				errorCallback(result);
			}

			successCallback();

		});
	}

	E_RESULT_CODE CBinaryFileWriter::Flush()
	{
		std::lock_guard<std::mutex> lock(mMutex);

		_getOutputStream()->Flush();
		return RC_OK;
	}

	E_RESULT_CODE CBinaryFileWriter::SetPosition(TSizeType pos)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		return mpStreamImpl->SetPosition(pos);
	}

	bool CBinaryFileWriter::IsEOF() const
	{
		std::lock_guard<std::mutex> lock(mMutex);

		return mpStreamImpl->IsEndOfStream();
	}

	CBinaryFileWriter::TSizeType CBinaryFileWriter::GetPosition() const
	{
		std::lock_guard<std::mutex> lock(mMutex);
		
		return mpStreamImpl->GetPosition();
	}

	E_RESULT_CODE CBinaryFileWriter::_onInit()
	{
		return _getOutputStream()->Reset(true);
	}

	E_RESULT_CODE CBinaryFileWriter::_onFree()
	{
		return RC_OK;
	}

	IOutputStream* CBinaryFileWriter::_getOutputStream()
	{
		if (!mpCachedOutputStream)
		{
			mpCachedOutputStream = dynamic_cast<IOutputStream*>(mpStreamImpl.Get());
		}

		return mpCachedOutputStream;
	}


	IFile* CreateBinaryFileWriter(IMountableStorage* pStorage, TPtr<IStream> pStream, E_RESULT_CODE& result)
	{
		CBinaryFileWriter* pFileInstance = new (std::nothrow) CBinaryFileWriter();

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