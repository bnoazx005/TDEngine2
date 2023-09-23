#include "./../../include/platform/CConfigFileWriter.h"
#include "./../../include/core/IFileSystem.h"
#include "../../include/platform/IOStreams.h"
#include "stringUtils.hpp"
#include <algorithm>
#include <cctype>


namespace TDEngine2
{
	CConfigFileWriter::CConfigFileWriter():
		CBaseFile(),
		mpCachedOutputStream(nullptr)
	{
	}

	E_RESULT_CODE CConfigFileWriter::SetInt(const std::string& group, const std::string& paramName, I32 value)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _writeValueToMap(group, paramName, std::to_string(value));
	}

	E_RESULT_CODE CConfigFileWriter::SetFloat(const std::string& group, const std::string& paramName, F32 value)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _writeValueToMap(group, paramName, std::to_string(value));
	}

	E_RESULT_CODE CConfigFileWriter::SetBool(const std::string& group, const std::string& paramName, bool value)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _writeValueToMap(group, paramName, std::to_string(value));
	}

	E_RESULT_CODE CConfigFileWriter::SetString(const std::string& group, const std::string& paramName, const std::string& value)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return _writeValueToMap(group, paramName, value);
	}

	E_RESULT_CODE CConfigFileWriter::_writeValueToMap(const std::string& group, const std::string& paramName, const std::string& value)
	{
		if (group.empty() || paramName.empty())
		{
			return RC_INVALID_ARGS;
		}

		mParamsMap[group][paramName] = value;

		return RC_OK;
	}

	E_RESULT_CODE CConfigFileWriter::_onFree()
	{
		thread_local const C8 lineSeparator = '\n';
		
		E_RESULT_CODE result = RC_OK;
		
		for (auto&& currGroup : mParamsMap)
		{
			const std::string groupName = Wrench::StringUtils::Format("[{0}]\n\n", currGroup.first);

			result = result | _getOutputStream()->Write(groupName.c_str(), groupName.size());

			for (auto&& currParamInfo : currGroup.second)
			{
				const std::string value = Wrench::StringUtils::Format("{0} = {1}\n", currParamInfo.first, currParamInfo.second);
				result = result | _getOutputStream()->Write(value.c_str(), value.size());
			}
		}

		return result;
	}

	IOutputStream* CConfigFileWriter::_getOutputStream()
	{
		if (!mpCachedOutputStream)
		{
			mpCachedOutputStream = dynamic_cast<IOutputStream*>(mpStreamImpl.Get());
		}

		return mpCachedOutputStream;
	}


	IFile* CreateConfigFileWriter(IMountableStorage* pStorage, TPtr<IStream> pStream, E_RESULT_CODE& result)
	{
		CConfigFileWriter* pFileInstance = new (std::nothrow) CConfigFileWriter();

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