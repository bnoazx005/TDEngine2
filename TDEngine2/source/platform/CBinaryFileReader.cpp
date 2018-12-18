#include "./../../include/platform/CBinaryFileReader.h"
#include "./../../include/core/IFileSystem.h"


namespace TDEngine2
{
	CBinaryFileReader::CBinaryFileReader():
		CBaseFile()
	{
		mCreationFlags = std::ios::in | std::ios::binary;
	}

	E_RESULT_CODE CBinaryFileReader::Read(U8*& pBuffer, U32 bufferSize)
	{
		return RC_OK;
	}

	E_RESULT_CODE CBinaryFileReader::SetPosition(U32 pos)
	{
		mFile.seekg(pos);

		return RC_OK;
	}

	bool CBinaryFileReader::IsEOF() const
	{
		return mFile.eof();
	}

	U32 CBinaryFileReader::GetPosition() const
	{
		return static_cast<U32>(mFile.tellg());
	}

	E_RESULT_CODE CBinaryFileReader::_onFree()
	{
		return RC_OK;
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