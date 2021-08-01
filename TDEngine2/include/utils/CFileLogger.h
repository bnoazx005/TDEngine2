/*!
	\file CFileLogger.h
	\date 25.09.2018
	\authors Kasimov Ildar
*/
#pragma once


#include "ILogger.h"
#include <fstream>
#include <mutex>


namespace TDEngine2
{
	/*!
		class CFileLogger

		\brief The class is an implementation of ILogger interface and represents
		a logger that writes down information into a file
	*/

	class CFileLogger:public ILogger
	{
		protected:
			enum E_LOG_MESSAGE_TYPE : U8
			{
				LMT_MESSAGE,
				LMT_WARNING,
				LMT_ERROR,
			};
		public:
			TDE2_API CFileLogger(const std::string& path);
			TDE2_API virtual ~CFileLogger();

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			TDE2_API void LogMessage(const std::string& message) override;

			TDE2_API void LogStatus(const std::string& message, const std::string& status, C8 delimiter, U16 delimitersCount);

			TDE2_API void LogError(const std::string& message) override;

			TDE2_API void LogWarning(const std::string& message) override;
		protected:
			TDE2_API CFileLogger();
			TDE2_API CFileLogger(const CFileLogger& builder) = delete;
			TDE2_API virtual CFileLogger& operator= (CFileLogger& builder) = delete;

			TDE2_API void _logMessageByType(E_LOG_MESSAGE_TYPE messageType, const std::string& message);
		protected:
			std::ofstream      mOutputLog;

			mutable std::mutex mMutex;
	};


	/// Global defined instance of a file logger
	/// CEngineCore's instance will free its memory by call Free method

	TDE2_API extern ILogger* MainLogger;


	/*!
		\brief The method just returns a pointer to global logger's object. This is just a wrapper
		for access to the global variable from another dlls, executables and etc
	*/

	TDE2_API ILogger* GetMainLogger();


	/*!
		\brief The macro definition to simplify logger's call. If the engine's code was built
		without DEBUG flag the macros will be skipped
	*/

#if TDE2_DEBUG_MODE
	#define LOG_DEBUG_MESSAGE(message)				\
		do											\
		{											\
			ILogger* pLogger = GetMainLogger();		\
			if (pLogger)							\
			{										\
				pLogger->LogMessage(message);		\
			}										\
		}											\
		while (false)
#else
	#define LOG_DEBUG_MESSAGE(message) do {} while(false)
#endif


	#define LOG_MESSAGE(message)					\
		do											\
		{											\
			ILogger* pLogger = GetMainLogger();		\
			if (pLogger)							\
			{										\
				pLogger->LogMessage(message);		\
			}										\
		}											\
		while (false)

	#define LOG_WARNING(message)					\
		do											\
		{											\
			ILogger* pLogger = GetMainLogger();		\
			if (pLogger)							\
			{										\
				pLogger->LogWarning(message);		\
			}										\
		}											\
		while (false)

	#define LOG_ERROR(message)						\
		do											\
		{											\
			ILogger* pLogger = GetMainLogger();		\
			if (pLogger)							\
			{										\
				pLogger->LogError(message);			\
			}										\
		}											\
		while (false)
}