#include "./../../include/platform/CTextFileReader.h"
#include<vector>
#include "./../../include/core/IFileSystem.h"
#include <sstream>


namespace TDEngine2
{
	CTextFileReader::CTextFileReader()
	{
	}
	
	std::string CTextFileReader::ReadLine()
	{
		if (!mFile.is_open())
		{
			return "";
		}

		std::string currReadLine;

		std::getline(mFile, currReadLine);

		return currReadLine;
	}

	std::string CTextFileReader::ReadToEnd()
	{
		if (!mFile.is_open())
		{
			return "";
		}

		if (!mCachedData.empty())
		{
			return mCachedData;
		}

		std::stringstream strBuffer;

		strBuffer << mFile.rdbuf();

		return (mCachedData = strBuffer.str());
	}

	E_RESULT_CODE CTextFileReader::_onFree()
	{
		return RC_OK;
	}


	IFile* CreateTextFileReader(IFileSystem* pFileSystem, const std::string& filename, E_RESULT_CODE& result)
	{
		CTextFileReader* pFileInstance = new (std::nothrow) CTextFileReader();

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