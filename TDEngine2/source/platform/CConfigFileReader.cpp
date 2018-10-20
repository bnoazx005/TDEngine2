#include "./../../include/platform/CConfigFileReader.h"
#include "./../../include/core/IFileSystem.h"
#include <algorithm>
#include <cctype>


namespace TDEngine2
{
	CConfigFileReader::CConfigFileReader()
	{
	}

	E_RESULT_CODE CConfigFileReader::Open(IFileSystem* pFileSystem, const std::string& filename)
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

	E_RESULT_CODE CConfigFileReader::Close()
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

	I32 CConfigFileReader::GetInt(const std::string& group, const std::string& paramName, I32 defaultValue)
	{
		std::string resultValue;

		if (_parseFileUntilParam(group, paramName, mParamsMap, resultValue) != RC_OK)
		{
			return defaultValue;
		}

		return std::atoi(resultValue.c_str());
	}

	F32 CConfigFileReader::GetFloat(const std::string& group, const std::string& paramName, F32 defaultValue)
	{
		std::string resultValue;

		if (_parseFileUntilParam(group, paramName, mParamsMap, resultValue) != RC_OK)
		{
			return defaultValue;
		}

		return std::atof(resultValue.c_str());
	}

	bool CConfigFileReader::GetBool(const std::string& group, const std::string& paramName, bool defaultValue)
	{
		std::string resultValue;

		if (_parseFileUntilParam(group, paramName, mParamsMap, resultValue) != RC_OK)
		{
			return defaultValue;
		}

		std::transform(resultValue.begin(), resultValue.end(), resultValue.begin(), std::tolower);

		return (resultValue == "true" || resultValue == "1") ? true : false;
	}
	
	std::string CConfigFileReader::GetString(const std::string& group, const std::string& paramName, std::string defaultValue)
	{
		std::string resultValue;

		if (_parseFileUntilParam(group, paramName, mParamsMap, resultValue) != RC_OK)
		{
			return defaultValue;
		}

		return resultValue;
	}

	std::string CConfigFileReader::GetFilename() const
	{
		return mName;
	}

	bool CConfigFileReader::IsOpen() const
	{
		return mFile.is_open();
	}

	E_RESULT_CODE CConfigFileReader::_parseFileUntilParam(const std::string& group, const std::string& paramName, TConfigParamsMap& paramsMap, std::string& value)
	{
		if (!mFile.is_open() || group.empty() || paramName.empty())
		{
			return RC_FAIL;
		}

		/// if needed entity already exists just return it
		if (!(value = paramsMap[group][paramName]).empty())
		{
			return RC_OK;
		}
				
		std::string currReadLine;

		U32 firstDelimPos  = 0;
		U32 secondDelimPos = 0;

		std::string currGroupName;
		std::string currParamName;
		std::string currValueName;

		while (std::getline(mFile, currReadLine))
		{
			if (currReadLine.empty())
			{
				continue;
			}

			currReadLine.erase(std::remove_if(currReadLine.begin(), currReadLine.end(), std::isspace), currReadLine.end()); ///< remove whitespaces

			if ((firstDelimPos = currReadLine.find_first_of('[')) != std::string::npos) /// found a group
			{
				if ((secondDelimPos = currReadLine.find_first_of(']')) == std::string::npos)
				{
					continue; /// just skip a line with an error
				}

				currGroupName = currReadLine.substr(firstDelimPos + 1, secondDelimPos - firstDelimPos - 1);

				continue;
			}
			
			if ((firstDelimPos = currReadLine.find_first_of('=')) != std::string::npos) /// found a parameter's declaration
			{
				currParamName = currReadLine.substr(0, firstDelimPos);
				currValueName = currReadLine.substr(firstDelimPos + 1, currReadLine.size() - firstDelimPos);

				if (!currParamName.empty() && !currParamName.empty())
				{
					paramsMap[currGroupName][currParamName] = currValueName;
				}

				if ((currGroupName == group) && (currParamName == paramName))
				{
					value = currValueName;

					return RC_OK;
				}
			}
		}

		return RC_OK;
	}


	IFileReader* CreateConfigFileReader(IFileSystem* pFileSystem, const std::string& filename, E_RESULT_CODE& result)
	{
		CConfigFileReader* pFileInstance = new (std::nothrow) CConfigFileReader();

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