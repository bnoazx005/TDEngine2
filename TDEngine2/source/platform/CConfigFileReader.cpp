#include "./../../include/platform/CConfigFileReader.h"
#include "./../../include/core/IFileSystem.h"
#include "../../include/platform/IOStreams.h"
#include <algorithm>
#include <cctype>


namespace TDEngine2
{
	CConfigFileReader::CConfigFileReader():
		mCurrParsingGroup()
	{
	}
	
	I32 CConfigFileReader::GetInt(const std::string& group, const std::string& paramName, I32 defaultValue)
	{
		std::string resultValue;

		if (_parseFileUntilParam(group, paramName, mParamsMap, resultValue) != RC_OK)
		{
			return defaultValue;
		}

		C8* ppStrEnd = nullptr;

		return static_cast<I32>(std::strtol(resultValue.c_str(), &ppStrEnd, 0));
	}

	F32 CConfigFileReader::GetFloat(const std::string& group, const std::string& paramName, F32 defaultValue)
	{
		std::string resultValue;

		if (_parseFileUntilParam(group, paramName, mParamsMap, resultValue) != RC_OK)
		{
			return defaultValue;
		}

		return static_cast<F32>(std::atof(resultValue.c_str()));
	}

	bool CConfigFileReader::GetBool(const std::string& group, const std::string& paramName, bool defaultValue)
	{
		std::string resultValue;

		if (_parseFileUntilParam(group, paramName, mParamsMap, resultValue) != RC_OK)
		{
			return defaultValue;
		}

		std::transform(resultValue.begin(), resultValue.end(), resultValue.begin(), [](U8 ch) { return std::tolower(ch); });

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

	E_RESULT_CODE CConfigFileReader::_parseFileUntilParam(const std::string& group, const std::string& paramName, TConfigParamsMap& paramsMap, std::string& value)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (!mpStreamImpl->IsValid() || group.empty() || paramName.empty())
		{
			return RC_FAIL;
		}

		/// if needed entity already exists just return it
		if (!(value = paramsMap[group][paramName]).empty())
		{
			return RC_OK;
		}
				
		std::string currReadLine;

		USIZE firstDelimPos  = 0;
		USIZE secondDelimPos = 0;

		std::string currGroupName { mCurrParsingGroup };
		std::string currParamName;
		std::string currValueName;

		TPtr<IInputStream> pInputStream = DynamicPtrCast<IInputStream>(mpStreamImpl);

		while (!mpStreamImpl->IsEndOfStream())
		{
			currReadLine = pInputStream->ReadLine();

			if (currReadLine.empty())
			{
				continue;
			}

			currReadLine = _removeComments(currReadLine);

			if ((firstDelimPos = currReadLine.find_first_of('[')) != std::string::npos) /// found a group
			{
				if ((secondDelimPos = currReadLine.find_first_of(']')) == std::string::npos)
				{
					continue; /// just skip a line with an error
				}

				mCurrParsingGroup = currReadLine.substr(firstDelimPos + 1, secondDelimPos - firstDelimPos - 1);
				currGroupName     = mCurrParsingGroup;

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

	E_RESULT_CODE CConfigFileReader::_onFree()
	{
		mCurrParsingGroup.clear();

		return RC_OK;
	}

	std::string CConfigFileReader::_removeComments(const std::string& line)
	{
		std::string processedLine(line);

		processedLine.erase(std::remove_if(processedLine.begin(), processedLine.end(), [](U8 ch) { return std::isspace(ch); }), processedLine.end()); ///< remove whitespaces

		/// remove single line comments
		std::string::size_type pos = processedLine.find('#');

		return processedLine.substr(0, pos);
	}


	IFile* CreateConfigFileReader(IMountableStorage* pStorage, TPtr<IStream> pStream, E_RESULT_CODE& result)
	{
		CConfigFileReader* pFileInstance = new (std::nothrow) CConfigFileReader();

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