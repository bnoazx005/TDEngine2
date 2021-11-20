#include "../../include/utils/CFileLogger.h"
#include "../../include/utils/Utils.h"
#include "../../include/editor/CPerfProfiler.h"
#include <iostream>
#include <thread>
#include <condition_variable>


namespace TDEngine2
{
	CFileLogger::CFileLogger(const std::string& path) :
		CBaseObject(), mIsOutputEnabled(true), mIsRunning(false)
	{
		mOutputLog.open(path.c_str());
	}

	E_RESULT_CODE CFileLogger::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsRunning = true;

		/// \note Main logger's thread
		std::thread([this] 
		{
			OPTICK_THREAD("FileLoggerThread");

			while (true)
			{
				{
					TDE2_PROFILER_SCOPE("FileLogger::Output");

					std::unique_lock<std::mutex> lock(mLogContext.mMutex);
					mLogContext.mQueueCondition.wait(lock, [this] { return !mIsRunning || !mLogContext.mMessages.empty(); });

					if (!mIsRunning)
					{
						return;
					}

					for (auto&& currMessage : mLogContext.mMessages)
					{
						_writeMessageInternal(currMessage);
					}

					mLogContext.mMessages.clear();
				}
			}
		}).detach();

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CFileLogger::_onFreeInternal()
	{
		{
			std::lock_guard<std::mutex> lock(mLogContext.mMutex);

			mIsRunning = false;
			mLogContext.mQueueCondition.notify_one();
		}

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

	void CFileLogger::SetOutputEnabled(bool value)
	{
		mIsOutputEnabled = value;
	}

	void CFileLogger::LogMessage(const std::string& message)
	{
		_logMessageByType(LMT_MESSAGE, message);
	}

	void CFileLogger::LogStatus(const std::string& message, const std::string& status, C8 delimiter, U16 delimitersCount)
	{
		std::stringstream outputMessageStream;

		outputMessageStream << message << " ";

		for (U16 i = 0; i < delimitersCount; ++i)
		{
			outputMessageStream << delimiter;
		}

		outputMessageStream << " " << status << std::endl;

		_logMessageByType(LMT_MESSAGE, outputMessageStream.str());
	}

	void CFileLogger::LogError(const std::string& message)
	{
		_logMessageByType(LMT_ERROR, message);
	}

	void CFileLogger::LogWarning(const std::string& message)
	{
		_logMessageByType(LMT_WARNING, message);
	}

#if TDE2_EDITORS_ENABLED

	void CFileLogger::OnBeforeMemoryRelease()
	{
	}

#endif

	void CFileLogger::_logMessageByType(E_LOG_MESSAGE_TYPE messageType, const std::string& message)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		static const std::unordered_map<E_LOG_MESSAGE_TYPE, std::string> messagePrefixesTable
		{
			{ LMT_MESSAGE, "" },
			{ LMT_WARNING, "!Warning: " },
			{ LMT_ERROR, "!!!ERROR: " },
		};

		if (!mOutputLog.is_open() || !mIsOutputEnabled)
		{
			return;
		}
		
		const std::string outputMessage = messagePrefixesTable.at(messageType) + message;

		/// \note The actual writing to the file is deffered and done in separate thread
		{
			std::lock_guard<std::mutex> lock(mLogContext.mMutex);

			mLogContext.mMessages.emplace_back(outputMessage);
			mLogContext.mQueueCondition.notify_one();
		}
	}

	void CFileLogger::_writeMessageInternal(const std::string& message)
	{
		mOutputLog << message << std::endl;
		mOutputLog.flush();

		std::cout << message << std::endl;
		std::cout.flush();
	}


	TDE2_API TPtr<ILogger> MainLogger = TPtr<ILogger>(new CFileLogger(MAIN_LOGGER_FILEPATH));


	TDE2_API TPtr<ILogger> GetMainLogger()
	{
		MainLogger->Init();
		return MainLogger;
	}
}