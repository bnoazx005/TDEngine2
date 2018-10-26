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
		#define TDE2_APIENTRY __cdecl					///< Calling convention for VS

		#if defined(TDE2_DLLIMPORT)
			#define TDE2_API __declspec(dllimport)
		#else
			#define TDE2_API __declspec(dllexport)
		#endif
	#elif defined(__GNUC__)
		#define TDE2_APIENTRY __attribute__((cdecl))	///< Calling convention for GNUC

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

		#define TDE2_BUILD_D3D11_GCTX_PLUGIN
		#define TDE2_BUILD_OGL_GCTX_PLUGIN
	#elif defined(__unix__) || defined(__unix) || defined(unix)
		#define TDE2_USE_UNIXPLATFORM

		#define TDE2_BUILD_OGL_GCTX_PLUGIN
	#else
	#endif

	#define GLEW_NO_GLU ///< Disable GLU 

	/// Main logger's settings
	#define MAIN_LOGGER_FILEPATH "TDEngine.log"


	/// Math configurable constants
	constexpr float FloatEpsilon = 1e-3f;
}