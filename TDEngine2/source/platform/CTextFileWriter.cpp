#include "./../../include/platform/CTextFileWriter.h"
#include "./../../include/core/IFileSystem.h"
#include "../../include/platform/IOStreams.h"


namespace TDEngine2
{
	CTextFileWriter::CTextFileWriter():
		mpCachedOutputStream(nullptr)
	{
	}
	
	E_RESULT_CODE CTextFileWriter::WriteLine(const std::string& value)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		thread_local const C8 lineSeparator = '\n';

		E_RESULT_CODE result = _getOutputStream()->Write(value.c_str(), value.size());
		result = result | _getOutputStream()->Write(&lineSeparator, sizeof(lineSeparator));

		return result;
	}

	E_RESULT_CODE CTextFileWriter::_onFree()
	{
		return RC_OK;
	}

	IOutputStream* CTextFileWriter::_getOutputStream()
	{
		if (!mpCachedOutputStream)
		{
			mpCachedOutputStream = dynamic_cast<IOutputStream*>(mpStreamImpl.Get());
		}

		return mpCachedOutputStream;
	}


	IFile* CreateTextFileWriter(IMountableStorage* pStorage, TPtr<IStream> pStream, E_RESULT_CODE& result)
	{
		CTextFileWriter* pFileInstance = new (std::nothrow) CTextFileWriter();

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