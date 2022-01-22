#include "./../../include/platform/CCsvFileReader.h"
#include "./../../include/core/IFileSystem.h"
#include "../../include/platform/IOStreams.h"
#include <sstream>
#include <vector>


namespace TDEngine2
{
	CCsvFileReader::CCsvFileReader():
		mpCachedInputStream(nullptr)
	{
	}
	
	E_RESULT_CODE CCsvFileReader::Open(IMountableStorage* pStorage, TPtr<IStream> pStream)
	{
		E_RESULT_CODE result = CBaseFile::Open(pStorage, pStream);
		if (result != RC_OK)
		{
			return result;
		}

		std::lock_guard<std::mutex> lock(mMutex);

		mIntermediateDataStream = std::stringstream(DynamicPtrCast<IInputStream>(mpStreamImpl)->ReadToEnd());

		mpCsvDocument = std::make_unique<rapidcsv::Document>(mIntermediateDataStream);
		TDE2_ASSERT(mpCsvDocument);

		if (!mpCsvDocument)
		{
			return RC_FAIL;
		}

		return result;
	}

	std::string CCsvFileReader::GetCellValue(const std::string& colId, U32 rowId)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return mpCsvDocument->GetCell<std::string>(colId, static_cast<USIZE>(rowId));
	}

	E_RESULT_CODE CCsvFileReader::_onFree()
	{
		return RC_OK;
	}

	IInputStream* CCsvFileReader::_getInputStream()
	{
		if (!mpCachedInputStream)
		{
			mpCachedInputStream = dynamic_cast<IInputStream*>(mpStreamImpl.Get());
		}

		return mpCachedInputStream;
	}


	IFile* CreateCsvFileReader(IMountableStorage* pStorage, TPtr<IStream> pStream, E_RESULT_CODE& result)
	{
		CCsvFileReader* pFileInstance = new (std::nothrow) CCsvFileReader();

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