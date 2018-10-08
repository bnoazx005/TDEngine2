#include "./../../include/platform/CTextFileReader.h"
#include<vector>
#include "./../../include/core/IFileSystem.h"


namespace TDEngine2
{
	CTextFileReader::CTextFileReader()
	{
	}

	E_RESULT_CODE CTextFileReader::Open(IFileSystem* pFileSystem, const std::string& filename)
	{
		if (!pFileSystem)
		{
			return RC_INVALID_ARGS;
		}

		if (mFile.is_open())
		{
			return RC_FAIL;
		}

		mFile.open(filename, std::ios::in);

		if (!mFile.is_open())
		{
			return RC_FILE_NOT_FOUND;
		}

		mName = filename;

		mpFileSystemInstance = pFileSystem;

		return RC_OK;
	}

	E_RESULT_CODE CTextFileReader::Close()
	{
		if (!mFile.is_open())
		{
			return RC_FAIL;
		}

		mFile.close();

		E_RESULT_CODE result = mpFileSystemInstance->CloseFile(this);

		if (result != RC_OK)
		{
			return result;
		}

		delete this;

		return RC_OK;
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

		mFile.seekg(0, std::ios::end);

		U32 fileLength = mFile.tellg();

		mFile.seekg(0, std::ios::beg);

		std::string textData;

		textData.resize(fileLength);
		
		mFile.read(&textData[0], fileLength);

		return textData;
	}

	std::string CTextFileReader::GetFilename() const
	{
		return mName;
	}

	bool CTextFileReader::IsOpen() const
	{
		return mFile.is_open();
	}

	IFileReader* CreateTextFileReader(IFileSystem* pFileSystem, const std::string& filename, E_RESULT_CODE& result)
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

		return dynamic_cast<IFileReader*>(pFileInstance);
	}
}