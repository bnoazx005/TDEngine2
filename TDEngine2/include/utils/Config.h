/*!
	\file Config.h
	\date 16.09.2018
	\authors Kasimov Ildar
*/


#pragma once


namespace TDEngine2
{
	/// Macroses' definitions to handle exporting objects

	#if defined(_WIN32) || defined(_MSC_VER)
		#if defined(TDE2_DLLIMPORT)
			#define TDE2_API __declspec(dllimport)
		#else
			#define TDE2_API __declspec(dllexport)
		#endif
	#elif defined(__GNUC__)	
		#if defined(TDE2_DLLIMPORT)
			#define TDE2_API 
		#else
			#define TDE2_API __attribute__((visibility("default")))
		#endif
	#else /// Unknown platform and compiler
		#define TDE2_API 
	#endif

	/// Platform-specific macroses are used to configure build in compile time

	#if defined(_WIN32)
		#define TDE2_USE_WIN32PLATFORM
	#endif

	/// Main logger's settings
	#define MAIN_LOGGER_FILEPATH "TDEngine.log"
}