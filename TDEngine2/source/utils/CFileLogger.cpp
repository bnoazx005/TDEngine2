#include "./../../include/utils/CFileLogger.h"


namespace TDEngine2
{
	CFileLogger::CFileLogger()
	{
	}

	CFileLogger::CFileLogger(const std::string& path)
	{
		mOutputLog.open(path.c_str());
	}

	CFileLogger::~CFileLogger()
	{
	}

	E_RESULT_CODE CFileLogger::Free()
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (!mOutputLog.is_open())
		{
			return RC_FAIL;
		}

		mOutputLog.close();
		
		return RC_OK;
	}

	void CFileLogger::LogMessage(const std::string& message)
	{
		_logMessageByType(LMT_MESSAGE, message);
	}

	void CFileLogger::LogError(const std::string& message)
	{
		_logMessageByType(LMT_ERROR, message);
	}

	void CFileLogger::LogWarning(const std::string& message)
	{
		_logMessageByType(LMT_WARNING, message);
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
				break;
			case LMT_ERROR:
				mOutputLog << "!!!ERROR: ";
				break;
		}

		mOutputLog << message << std::endl;

		mOutputLog.flush();
	}


	ILogger* CreateFileLogger(const std::string& path, E_RESULT_CODE& result)
	{
		ILogger* pLoggerInstance = new (std::nothrow) CFileLogger(path);

		if (!pLoggerInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = RC_OK;

		return pLoggerInstance;
	}

	ILogger* MainLogger = new CFileLogger(MAIN_LOGGER_FILEPATH);
}