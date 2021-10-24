#include "./../../include/platform/CTextFileReader.h"
#include "./../../include/core/IFileSystem.h"
#include "../../include/platform/IOStreams.h"
#include <sstream>
#include <vector>


namespace TDEngine2
{
	CTextFileReader::CTextFileReader():
		mpCachedInputStream(nullptr)
	{
	}
	
	std::string CTextFileReader::ReadLine()
	{
		std::lock_guard<std::mutex> lock(mMutex);

		return _getInputStream()->ReadLine();
	}

	std::string CTextFileReader::ReadToEnd()
	{
		std::lock_guard<std::mutex> lock(mMutex);

		return _getInputStream()->ReadToEnd();
	}

	E_RESULT_CODE CTextFileReader::_onFree()
	{
		return RC_OK;
	}

	IInputStream* CTextFileReader::_getInputStream()
	{
		if (!mpCachedInputStream)
		{
			mpCachedInputStream = dynamic_cast<IInputStream*>(mpStreamImpl.Get());
		}

		return mpCachedInputStream;
	}


	IFile* CreateTextFileReader(IMountableStorage* pStorage, TPtr<IStream> pStream, E_RESULT_CODE& result)
	{
		CTextFileReader* pFileInstance = new (std::nothrow) CTextFileReader();

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