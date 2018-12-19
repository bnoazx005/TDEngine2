#include "./../../include/platform/CBinaryFileWriter.h"
#include "./../../include/core/IFileSystem.h"


namespace TDEngine2
{
	CBinaryFileWriter::CBinaryFileWriter() :
		CBaseFile()
	{
		mCreationFlags = std::ios::out | std::ios::binary;
	}

	E_RESULT_CODE CBinaryFileWriter::Write(const void* pBuffer, U32 bufferSize)
	{
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

	E_RESULT_CODE CBinaryFileWriter::Flush()
	{
		mFile.flush();

		if (mFile.bad())
		{
			return RC_FAIL;
		}

		return RC_OK;
	}

	E_RESULT_CODE CBinaryFileWriter::SetPosition(U32 pos)
	{
		mFile.seekg(pos);

		return RC_OK;
	}

	bool CBinaryFileWriter::IsEOF() const
	{
		return mFile.eof();
	}

	U32 CBinaryFileWriter::GetPosition() const
	{
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