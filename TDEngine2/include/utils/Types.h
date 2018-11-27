/*!
	\file Types.h
	\date 14.09.2018
	\authors Kasimov Ildar
*/


#pragma once


#include "Config.h"
#include <cstdint>
#include <limits>
#include <string>
#include <GL/glew.h>

#if _HAS_CXX17	/// use variant only if C++17's implementation is available, otherwise use stardard unions
	#include <variant>
#endif

/// include platform specific headers
#if defined (TDE2_USE_WIN32PLATFORM)
	#include <Windows.h>
	#include <d3d11.h>
#elif defined (TDE2_USE_UNIXPLATFORM)
	#include <X11/Xlib.h>
#else
#endif

#include <vector>
#include <unordered_map>


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


	typedef U32 TypeId;

	constexpr TypeId InvalidTypeId = 0;


	/// Pointer type
	typedef uintptr_t U32Ptr;

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
		RC_FILE_NOT_FOUND,				/// The engine can't find specified file in a file system
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
		EST_FILE_SYSTEM,				/// A subsystem represents a virtual file system that the engine uses
		EST_RESOURCE_MANAGER,			/// A subsystem represents root resource manager
		EST_JOB_MANAGER,				/// A subsytem represents a thread pool
		EST_PLUGIN_MANAGER,				/// A subsystem represents a plugin manager
		EST_EVENT_MANAGER,				/// A subsystem provides a centralized event bus
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
#elif defined (TDE2_USE_UNIXPLATFORM)

	/*!
		struct TUnixInternalWindowData

		\brief The structure contains handlers that are used to work with a window
	*/

	typedef struct TUnixInternalWindowData
	{
		Display* mpDisplayHandler;
		Window   mRootWindowHandler;
		Window   mWindowHandler;
		I32      mScreenId;
	} TUnixInternalWindowData;

	typedef TUnixInternalWindowData TWindowSystemInternalData; 
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

#elif defined (TDE2_USE_UNIXPLATFORM)
	/*!
		struct TOGLCtxInternalData

		\brief The structure contains handlers of graphics context that are associated with OGL system
	*/

	typedef struct TOGLInternalData
	{
		/// \todo preliminary declaration of a type, should be completed later 
	} TOGLInternalData, *TOGLInternalDataPtr;

	typedef TOGLInternalData TGraphicsCtxInternalData;
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
#elif defined (TDE2_USE_UNIXPLATFORM)
	/*!
		union TBufferInternalData

		\brief The union contains low-level handlers to a buffer depending on used GAPI
	*/

	typedef struct TBufferInternalData
	{
		GLuint mGLBuffer;
	} TBufferInternalData, *TBufferInternalDataPtr;
#else
#endif


	/// Entity-Component-System's types declarations


	typedef U32 TEntityId; ///< A type of entity's identifier

	constexpr TEntityId InvalidEntityId = (std::numeric_limits<TEntityId>::max)(); ///< Invalid value for TEntityId type

	
	typedef U32 TComponentTypeId; ///< A type of a component

	constexpr TComponentTypeId InvalidComponentType = (std::numeric_limits<TEntityId>::max)(); ///< Invalid value for TComponentTypeId type


	typedef U32 TComponentFactoryId; 

	constexpr TComponentFactoryId InvalidComponentFactoryId = 0;


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

	
	typedef U32 TDynamicLibraryHandler; ///< A type of dynamic library's handler

	constexpr TDynamicLibraryHandler InvalidDynamicLibHandlerValue = (std::numeric_limits<TDynamicLibraryHandler>::max)();

#if defined (TDE2_USE_WIN32PLATFORM)
	typedef HMODULE TDynamicLibrary;
#elif defined (TDE2_USE_WIN32PLATFORM)
	typedef void* TDynamicLibrary;
#else
	typedef void* TDynamicLibrary;
#endif


	/*!
		enumeration E_VERTEX_ELEMENT_SEMANTIC_TYPE

		\brief E_VERTEX_ELEMENT_SEMANTIC_TYPE is an enumeration that contains all types, which can be assigned to
		a single vertex's element and used by a shader program (especially for D3D11)
	*/

	enum E_VERTEX_ELEMENT_SEMANTIC_TYPE: U32
	{
		VEST_POSITION,
		VEST_NORMAL,
		VEST_TEXCOORDS,
		VEST_BINORMAL,
		VEST_TANGENT,
		VEST_COLOR,
		VEST_UNKNOWN
	};


	/*!
		enumeration E_FORMAT_TYPE

		\brief The enumeration contains all available formats, which are used by the engine
	*/

	enum E_FORMAT_TYPE: U32
	{
		FT_FLOAT1,
		FT_FLOAT2,
		FT_FLOAT3,
		FT_FLOAT4,
		FT_SHORT1,
		FT_SHORT2,
		FT_SHORT3,
		FT_SHORT4,
		FT_USHORT1,
		FT_USHORT2,
		FT_USHORT3,
		FT_USHORT4,
		FT_UINT1,
		FT_UINT2,
		FT_UINT3,
		FT_UINT4,
		FT_SINT1,
		FT_SINT2,
		FT_SINT3,
		FT_SINT4,
		FT_UBYTE1,
		FT_UBYTE2,
		FT_UBYTE3,
		FT_UBYTE4,
		FT_BYTE1,
		FT_BYTE2,
		FT_BYTE3,
		FT_BYTE4,
		FT_NORM_SHORT1,
		FT_NORM_SHORT2,
		FT_NORM_SHORT3,
		FT_NORM_SHORT4,
		FT_NORM_USHORT1,
		FT_NORM_USHORT2,
		FT_NORM_USHORT3,
		FT_NORM_USHORT4,
		FT_NORM_UINT1,
		FT_NORM_UINT2,
		FT_NORM_UINT3,
		FT_NORM_UINT4,
		FT_NORM_SINT1,
		FT_NORM_SINT2,
		FT_NORM_SINT3,
		FT_NORM_SINT4,
		FT_NORM_UBYTE1,
		FT_NORM_UBYTE2,
		FT_NORM_UBYTE3,
		FT_NORM_UBYTE4,
		FT_NORM_BYTE1,
		FT_NORM_BYTE2,
		FT_NORM_BYTE3,
		FT_NORM_BYTE4_SRGB,
		FT_NORM_BYTE4,
		FT_D32,
		FT_FLOAT1_TYPELESS,
		FT_FLOAT2_TYPELESS,
		FT_FLOAT3_TYPELESS,
		FT_FLOAT4_TYPELESS,
		FT_UBYTE4_BGRA_UNORM,
		FT_UNKNOWN
	};


	/*!
		\brief Resource manager's types section
	*/

	typedef U32 TResourceId; ///< Unique resource identifier that is used for a cross-referencing

	constexpr TResourceId InvalidResourceId = 0;

	/*!
		enum E_RESOURCE_STATE_TYPE

		\brief The enumeration contains all possible states' types, in which a resource can stay
	*/

	enum E_RESOURCE_STATE_TYPE : U8
	{
		RST_LOADING,		///< A resource is loading at the time
		RST_LOADED,			///< A resource is loaded and ready to use
		RST_UNLOADED,		///< A resource is unloaded from memory, but can be loaded again
		RST_PENDING,		///< An initial state of any resource
	};


	typedef U32 TResourceLoaderId; ///< A resource loader's identifier

	constexpr TResourceLoaderId InvalidResourceLoaderId = 0;


	/*!
		structure TRegisterLoaderResult

		\brief The structure is used as compound result type of RegisterLoader method of IResourceManager
	*/

	typedef struct TRegisterLoaderResult
	{
		E_RESULT_CODE                      mResultCode;

		TResourceLoaderId                  mResourceLoaderId;

		static const TRegisterLoaderResult mInvalidValue;
	} TRegisterLoaderResult;


	typedef U32 TResourceFactoryId; ///< A resource factory's identifier

	constexpr TResourceFactoryId InvalidResourceFactoryId = 0;


	/*!
		structure TRegisterFactoryResult

		\brief The structure is used as compound result type of RegisterFactory method of IResourceManager
	*/

	typedef struct TRegisterFactoryResult
	{
		E_RESULT_CODE                       mResultCode;

		TResourceFactoryId                  mResourceFactoryId;

		static const TRegisterFactoryResult mInvalidValue;
	} TRegisterFactoryResult;


	/*!
		enum E_RESOURCE_ACCESS_TYPE

		\brief The enumeration contains types of a resource's access
	*/

	enum E_RESOURCE_ACCESS_TYPE : U8
	{
		RAT_BLOCKING,			///< An executation within a particular thread will be freezed until a resource will be loaded
		RAT_STREAMING,			///< A resource's loading will be executed asynchronously, if the resource isn't loaded yet the default variant of it will be returned and updated later
	};


	/*!
		enumeration E_SHADER_STAGE_TYPE

		\brief The enumeration contains types of shaders
	*/

	enum E_SHADER_STAGE_TYPE
	{
		SST_VERTEX,
		SST_PIXEL,
		SST_GEOMETRY
	};

	/*!
		enum E_SHADER_FEATURE_LEVEL

		\brief The enumeration contains a list of available shaders feature sets
	*/

	enum E_SHADER_FEATURE_LEVEL
	{
		SFL_3_0,	///< Corresponds to SM 3.0 (D3D) and 150 (GL)
		SFL_4_0,	///< Corresponds to SM 4.0 (D3D) and 330 (GL)
		SFL_5_0,	///< Corresponds to SM 5.0 (D3D) and 420 (GL)
	};


	/*!
		enum E_INTERNAL_UNIFORM_BUFFER_REGISTERS

		\brief The enumeration contains all available registers of uniforms buffers
		that are marked for internal usage by the engine
	*/

	enum E_INTERNAL_UNIFORM_BUFFER_REGISTERS : U8
	{
		IUBR_PER_FRAME,			///< This uniforms buffer is updated each frame
		IUBR_PER_OBJECT,		///< This uniforms buffer is unique for each model
		IUBR_RARE_UDATED,		///< This uniforms buffer contains rare updating values 
		IUBR_CONSTANTS,			///< This uniforms buffer contains in-engine constants (like Pi, Epsilon, etc)
		IUBR_LAST_USED_SLOT = IUBR_CONSTANTS
	};

	constexpr U8 MaxNumberOfUserConstantBuffers = 10;


	typedef U32 TEventListenerId;

	constexpr TEventListenerId BroadcastListenersIdValue = (std::numeric_limits<TEventListenerId>::max)();
}