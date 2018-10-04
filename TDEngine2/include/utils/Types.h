/*!
	\file Types.h
	\date 14.09.2018
	\authors Kasimov Ildar
*/


#pragma once


#include "Config.h"
#include <cstdint>
#include <limits>

#if _HAS_CXX17	/// use variant only if C++17's implementation is available, otherwise use stardard unions
	#include <variant>
#endif

/// include platform specific headers
#if defined (TDE2_USE_WIN32PLATFORM)
	#include <Windows.h>
	#include <d3d11.h>
	#include <gl/glew.h>
#else
#endif


namespace TDEngine2
{	
	/// Integral types

	typedef int8_t   I8;
	typedef int16_t  I16;
	typedef int32_t  I32;
	typedef int64_t  I64;
	typedef uint8_t  U8;
	typedef uint16_t U16;
	typedef uint32_t U32;
	typedef uint64_t U64;

	/// Floating-point types

	typedef float  F32;
	typedef double F64;

	/// Symbolic types

	typedef char     C8;
	typedef char16_t C16;
	typedef char32_t C32;

	/// Error codes

	/*!
		enum E_RESULT_CODE

		\brief The enumeration contains all the codes that can be returned as execution's result
	*/

	enum E_RESULT_CODE : U32
	{
		RC_OK,							/// Normal execution
		RC_FAIL,						/// An execution failed at some point
		RC_INVALID_ARGS,				/// Some of input arguments are invalid or incorrect
		RC_OUT_OF_MEMORY,				/// Couldn't allocate enough memory to store an object
		RC_NOT_IMPLEMENTED_YET,			/// A function or a method has no implementation yet
		RC_GAPI_IS_NOT_SUPPORTED,		/// A GAPI is not supported on a platform
		RC_PLUGIN_IS_NOT_SUPPORTED,		/// Incompatible version of a plugin, or it was built with different version of the engine's library
		RC_UNKNOWN						/// Some unrecognized error
	};


	/*!
		enum E_ENGINE_SUBSYSTEM_TYPE

		\brief The enumeration contains all types of subsystems that can be used within the engine
	*/

	enum E_ENGINE_SUBSYSTEM_TYPE: U8
	{
		EST_WINDOW,						/// A subsystem that responsible for windows' management (creation, etc)
		EST_GRAPHICS_CONTEXT,			/// A subsystem represents a low-level graphics layer (wrappers for D3D, OGL, etc)
		//EST_FILE_SYSTEM,
		EST_UNKNOWN						/// Unused value, but can be helpful if some user wants to know the amount of available subsystems
	};


	/*!
		enum E_GRAPHICS_CONTEXT__GAPI_TYPE

		\brief The enumeration contains all avaiable GAPI that existing graphics context supports
	*/

	enum E_GRAPHICS_CONTEXT_GAPI_TYPE: U8
	{
		GCGT_DIRECT3D11,	/// Direct3D 11.0 and above
		GCGT_OPENGL3X,		/// OpenGL of version 3.0 and above
		GCGT_UNKNOWN
	};


	/*!
		enum E_PARAMETERS

		\brief The enumeration is a list of available features that could be enabled/disabled
	*/

	enum E_PARAMETERS: U8
	{
		P_FULLSCREEN                = 0x1,				/// Enables full-screen mode
		P_VSYNC                     = 0x2,				/// Enables vertical synchronization
		P_RESIZEABLE                = 0x4,				/// Determines will be a window resizeable or not
		P_HARDWARE_GAMMA_CORRECTION = 0x8,				/// Enables a hardware gamma correction (sRGB back buffer) if it's supported on by a video card
	};


	/// Try to infer a type TWindowSystemInternalData, that should be used to store internal window's data based on specific platform

#if defined(TDE2_USE_WIN32PLATFORM)

	/*!
		struct TWin32InternalWindowData

		\brief The structure contains handlers that are used to work with a window
	*/

	typedef struct TWin32InternalWindowData
	{
		HWND      mWindowHandler;
		HINSTANCE mWindowInstanceHandler;
		HDC       mDeviceContextHandler;
	} TWin32InternalWindowData;

	typedef TWin32InternalWindowData TWindowSystemInternalData; 
#else
	typedef void* TWindowSystemInternalData;
#endif


	/// Try to infer a type TGraphicsCtxInternalData 

#if defined(TDE2_USE_WIN32PLATFORM)


	/*!
		struct TD3D11CtxInternalData

		\brief The structure contains pointers to ID3D11Device and ID3D11DeviceContext, that are used
		to work with graphics
	*/

	typedef struct TD3D11CtxInternalData
	{
		ID3D11Device*        mp3dDevice;
		ID3D11DeviceContext* mp3dDeviceContext;
	} TD3D11CtxInternalData, *TD3D11CtxInternalDataPtr;


	/*!
		struct TOGLCtxInternalData

		\brief The structure contains handlers of graphics context that are associated with OGL system
	*/

	typedef struct TOGLInternalData
	{
		/// \todo preliminary declaration of a type, should be completed later 
	} TOGLInternalData, *TOGLInternalDataPtr;

	#if _HAS_CXX17
		typedef std::variant<TD3D11CtxInternalData, TOGLInternalData> TGraphicsCtxInternalData;
	#else
		typedef union TGraphicsCtxInternalData
		{
			TD3D11CtxInternalData mD3D11;
			TOGLInternalData      mOGL;
		} TGraphicsCtxInternalData, *TGraphicsCtxInternalDataPtr;
	#endif

#else
#endif


	/// Try to infer a type TGraphicsCtxInternalData 
#if defined (TDE2_USE_WIN32PLATFORM)
	/*!
		union TBufferInternalData

		\brief The union contains low-level handlers to a buffer depending on used GAPI
	*/

	typedef union TBufferInternalData
	{
		ID3D11Buffer* mpD3D11Buffer;
		GLuint        mGLBuffer;
	} TBufferInternalData, *TBufferInternalDataPtr;

#else
#endif


	/// Entity-Component-System's types declarations


	typedef U32 TEntityId; ///< A type of entity's identifier

	constexpr TEntityId InvalidEntityId = (std::numeric_limits<TEntityId>::max)(); ///< Invalid value for TEntityId type

	
	typedef U32 TComponentTypeId; ///< A type of a component

	constexpr TComponentTypeId InvalidComponentType = (std::numeric_limits<TEntityId>::max)(); ///< Invalid value for TComponentTypeId type


	/*!
		structure TPluginInfo

		\brief The structure contains an information about a plugin
	*/

	typedef struct TPluginInfo
	{
		std::string mName;				///< A name of a plugin
		U32         mPluginVersion;		///< Plugin's version low 16 high bits refer to a major part, low 16 bits refer to a minor part
		U32         mEngineVersion;		///< A version of the engine, which a plugin uses
	} TPluginInfo, *TPluginInfoPtr;


#if defined (TDE2_USE_WIN32PLATFORM)
	typedef HMODULE TSharedLibraryHandler;
#else
	typedef void*   TSharedLibraryHandler;
#endif
}