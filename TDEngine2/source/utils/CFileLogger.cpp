#include "../../include/utils/CFileLogger.h"
#include "../../include/utils/Utils.h"
#include <iostream>


namespace TDEngine2
{
	CFileLogger::CFileLogger(const std::string& path)
	{
		mOutputLog.open(path.c_str());
	}

	E_RESULT_CODE CFileLogger::_onFreeInternal()
	{
		if (!mOutputLog.is_open())
		{
			return RC_FAIL;
		}

		mOutputLog.close();
		
		return RC_OK;
	}

#if TDE2_EDITORS_ENABLED

	void CFileLogger::_onBeforeMemoryRelease()
	{
	}

#endif


	void CFileLogger::LogMessage(const std::string& message)
	{
		_logMessageByType(LMT_MESSAGE, message);
	}

	void CFileLogger::LogStatus(const std::string& message, const std::string& status, C8 delimiter, U16 delimitersCount)
	{
		mOutputLog << message << " ";
		std::cout << message << " ";

		for (U16 i = 0; i < delimitersCount; ++i)
		{
			mOutputLog << delimiter;
			std::cout << delimiter;
		}

		mOutputLog << " " << status << std::endl;
		std::cout << " " << status << std::endl;
	}

	void CFileLogger::LogError(const std::string& message)
	{
		_logMessageByType(LMT_ERROR, message);
	}

	void CFileLogger::LogWarning(const std::string& message)
	{
		_logMessageByType(LMT_WARNING, message);
	}

	void CFileLogger::OnBeforeMemoryRelease()
	{
	}

	void CFileLogger::_logMessageByType(E_LOG_MESSAGE_TYPE messageType, const std::string& message)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (!mOutputLog.is_open())
		{
			return;
		}
		
		switch (messageType)
		{
			case LMT_MESSAGE:	/// just do nothing
				break;
			case LMT_WARNING:
				mOutputLog << "!Warning: ";
				std::cout << "!Warning: ";
				break;
			case LMT_ERROR:
				mOutputLog << "!!!ERROR: ";
				std::cout << "!!!ERROR: ";
				break;
		}

		mOutputLog << message << std::endl;
		std::cout << message << std::endl;

		mOutputLog.flush();
		std::cout.flush();
	}


	TDE2_API TPtr<ILogger> MainLogger = TPtr<ILogger>(new CFileLogger(MAIN_LOGGER_FILEPATH));


	TDE2_API TPtr<ILogger> GetMainLogger()
	{
		return MainLogger;
	}
}