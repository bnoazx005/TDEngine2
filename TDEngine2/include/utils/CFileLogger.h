/*!
	\file CFileLogger.h
	\date 25.09.2018
	\authors Kasimov Ildar
*/
#pragma once


#include "ILogger.h"
#include <fstream>


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

			TDE2_API void LogError(const std::string& message) override;

			TDE2_API void LogWarning(const std::string& message) override;
		protected:
			TDE2_API CFileLogger();
			TDE2_API CFileLogger(const CFileLogger& builder) = delete;
			TDE2_API virtual CFileLogger& operator= (CFileLogger& builder) = delete;

			TDE2_API void _logMessageByType(E_LOG_MESSAGE_TYPE messageType, const std::string& message);
		protected:
			std::ofstream mOutputLog;
	};


	/// Global defined instance of a file logger
	/// CEngineCore's instance will free its memory by call Free method

	TDE2_API extern ILogger* MainLogger;


	/*!
		\brief The macro definition to simplify logger's call. If the engine's code was built
		without DEBUG flag the macros will be skipped
	*/

#if defined(_DEBUG)

	#define LOG_MESSAGE(message)				\
		if (MainLogger)							\
		{										\
			MainLogger->LogMessage(message);	\
		}										

	#define LOG_WARNING(message)				\
		if (MainLogger)							\
		{										\
			MainLogger->LogWarning(message);	\
		}									

	#define LOG_ERROR(message)					\
		if (MainLogger)							\
		{										\
			MainLogger->LogError(message);		\
		}										
#else
	#define LOG_MESSAGE(message)
	#define LOG_WARNING(message) 
	#define LOG_ERROR(message)
#endif
}