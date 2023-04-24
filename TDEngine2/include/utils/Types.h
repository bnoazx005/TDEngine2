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
#if defined (TDE2_USE_WINPLATFORM)
	#include <Windows.h>
	#include <d3d11.h>
#elif defined (TDE2_USE_UNIXPLATFORM)
	struct _XDisplay;
	typedef unsigned long TX11WindowType;
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
	typedef size_t   USIZE;

	/// Floating-point types

	typedef float  F32;
	typedef double F64;

	/// Symbolic types

	typedef char     C8;
	typedef char16_t C16;
	typedef char32_t C32;


	/// Pointer type
	typedef uintptr_t U32Ptr;


#define ENUM_META()
#define CLASS_META()


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
		RC_ASYNC_FILE_IO_IS_DISABLED,	/// The code means that asyncronous file I/O is not available 
		RC_INVALID_FILE,				/// The code means that some reader has tried to read file, but something went wrong
		RC_UNKNOWN						/// Some unrecognized error
	};


	/*!
		enum E_ENGINE_SUBSYSTEM_TYPE

		\brief The enumeration contains all types of subsystems that can be used within the engine
	*/

	ENUM_META()
	enum E_ENGINE_SUBSYSTEM_TYPE: U8
	{
		EST_WINDOW,						/// A subsystem that responsible for windows' management (creation, etc)
		EST_GRAPHICS_CONTEXT,			/// A subsystem represents a low-level graphics layer (wrappers for D3D, OGL, etc)
		EST_FILE_SYSTEM,				/// A subsystem represents a virtual file system that the engine uses
		EST_RESOURCE_MANAGER,			/// A subsystem represents root resource manager
		EST_JOB_MANAGER,				/// A subsystem represents a thread pool
		EST_PLUGIN_MANAGER,				/// A subsystem represents a plugin manager
		EST_EVENT_MANAGER,				/// A subsystem provides a centralized event bus
		EST_RENDERER,					/// A subsystem represents a renderer
		EST_INPUT_CONTEXT,				/// A subsystem represents a low-level input system
		EST_IMGUI_CONTEXT,				/// A subsystem represents an immediate mode GUI system
		EST_EDITORS_MANAGER,            /// A subsystem that operates all editors that the engine provides for users (Available only in debug and production builds)
		EST_SCENE_MANAGER,				/// A subsystem that operates with scenes (load, unload them)
		EST_LOCALIZATION_MANAGER,		/// A subsystem that implements a manager of locales
		EST_SAVE_MANAGER,				/// A subsystem that provides work with game saves
		EST_AUDIO_CONTEXT,				/// 
		EST_GAME_MODES_MANAGER,			/// 
		EST_UNKNOWN						/// Unused value, but can be helpful if some user wants to know the amount of available subsystems
	};


	/*!
		enum E_GRAPHICS_CONTEXT__GAPI_TYPE

		\brief The enumeration contains all avaiable GAPI that existing graphics context supports
	*/

	ENUM_META() enum E_GRAPHICS_CONTEXT_GAPI_TYPE: U8
	{
		GCGT_DIRECT3D11,	/// Direct3D 11.0 and above
		GCGT_OPENGL3X,		/// OpenGL of version 3.0 and above
		GCGT_UNKNOWN
	};


	/*!
		enum E_AUDIO_CONTEXT_API_TYPE

		\brief The enumeration contains all avaiable types for audio context. For now only FMOD middle-ware is supported
	*/

	enum class E_AUDIO_CONTEXT_API_TYPE : U8
	{
		FMOD,
		UNKNOWN
	};


	/*!
		enum E_PARAMETERS

		\brief The enumeration is a list of available features that could be enabled/disabled
	*/

	enum E_PARAMETERS: U8
	{
		P_FULLSCREEN                = 0x1,				///< Enables full-screen mode
		P_VSYNC                     = 0x2,				///< Enables vertical synchronization
		P_RESIZEABLE                = 0x4,				///< Determines will be a window resizeable or not
		P_HARDWARE_GAMMA_CORRECTION = 0x8,				///< Enables a hardware gamma correction (sRGB back buffer) if it's supported on by a video card
		P_ZBUFFER_ENABLED           = 0x10,				///< Enables a depth buffer (it's disabled by default)
		P_WINDOWLESS_MODE			= 0x20,				///< Switch engine's behaviour into console-ish mode (you can't work with renderer and window systems in it)
	};


	/// Try to infer a type TWindowSystemInternalData, that should be used to store internal window's data based on specific platform

#if defined(TDE2_USE_WINPLATFORM)

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
		_XDisplay*     mpDisplayHandler;
		TX11WindowType mRootWindowHandler;
		TX11WindowType mWindowHandler;
		I32            mScreenId;
	} TUnixInternalWindowData;

	typedef TUnixInternalWindowData TWindowSystemInternalData; 
#else
	typedef void* TWindowSystemInternalData;
#endif


	/// Try to infer a type TGraphicsCtxInternalData 

#if defined(TDE2_USE_WINPLATFORM)


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
#if defined (TDE2_USE_WINPLATFORM)
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


	template <typename T> std::string ToString(const T& value);

	/// \note Default ToString specializations

	template<> inline std::string ToString<std::string>(const std::string& value) { return value; }
	template<> inline std::string ToString<bool>(const bool& value) { return value ? "True" : "False"; }
	template<> inline std::string ToString<I32>(const I32& value) { return std::to_string(value); }

	/*!
		\brief The macro TDE2_DECLARE_HANDLE_TYPE is used to declare strongly typed handles instead of using typedefs 
	*/

#define TDE2_DECLARE_HANDLE_TYPE_EX(HandleID, InvalidValue) \
	enum class HandleID : U32 { Invalid = InvalidValue };	\
	template<> inline std::string ToString<HandleID>(const HandleID& value) { return std::string(#HandleID).append("(").append(std::to_string(static_cast<U32>(value))).append(")"); }

#define TDE2_DECLARE_HANDLE_TYPE(HandleID) TDE2_DECLARE_HANDLE_TYPE_EX(HandleID, (std::numeric_limits<U32>::max)())


	TDE2_DECLARE_HANDLE_TYPE_EX(TypeId, 0);
	TDE2_DECLARE_HANDLE_TYPE(TEngineListenerId);


	/// Entity-Component-System's types declarations


	TDE2_DECLARE_HANDLE_TYPE(TEntityId); ///< A type of entity's identifier	
	TDE2_DECLARE_HANDLE_TYPE(TSystemId);


	/*!
		enum class E_SYSTEM_PRIORITY

		\brief The enumeration contains all allowed priorities values
	*/

	enum class E_SYSTEM_PRIORITY : U8
	{
		SP_LOW_PRIORITY = 0x8,
		SP_NORMAL_PRIORITY = 0x4,
		SP_HIGH_PRIORITY = 0x2,
		SP_HIGHEST_PRIORITY = 0x1
	};


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

	
	TDE2_DECLARE_HANDLE_TYPE(TDynamicLibraryHandler); ///< A type of dynamic library's handler

#if defined (TDE2_USE_WINPLATFORM)
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
		VEST_JOINT_INDICES,
		VEST_JOINT_WEIGHTS,
		VEST_UNKNOWN
	};


	/*!
		enumeration E_FORMAT_TYPE

		\brief The enumeration contains all available formats, which are used by the engine
	*/

	ENUM_META()
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

	TDE2_DECLARE_HANDLE_TYPE(TResourceId); ///< Unique resource identifier that is used for a cross-referencing

	/*!
		enum E_RESOURCE_STATE_TYPE

		\brief The enumeration contains all possible states' types, in which a resource can stay
	*/

	enum class E_RESOURCE_STATE_TYPE : U8
	{
		RST_LOADING,		///< A resource is loading at the time
		RST_LOADED,			///< A resource is loaded and ready to use
		RST_UNLOADED,		///< A resource is unloaded from memory, but can be loaded again
		RST_PENDING,		///< An initial state of any resource
		RST_DESTROYING,		///< A state of a resource between already unloaded, but not removed from the memory
	};


	/*!
		enum class E_RESOURCE_LOADING_POLICY

		\brief The space of flags that defines whether a resource should be loaded in blocking manner or not
	*/

	enum class E_RESOURCE_LOADING_POLICY : U8
	{
		SYNCED,
		STREAMING,
		DEFAULT
	};


	TDE2_DECLARE_HANDLE_TYPE_EX(TResourceLoaderId, 0); ///< A resource loader's identifier
	TDE2_DECLARE_HANDLE_TYPE_EX(TResourceFactoryId, 0); ///< A resource factory's identifier


	/*!
		enumeration E_SHADER_STAGE_TYPE

		\brief The enumeration contains types of shaders
	*/

	enum E_SHADER_STAGE_TYPE: U8
	{
		SST_VERTEX,
		SST_PIXEL,
		SST_GEOMETRY,
		SST_COMPUTE,
		SST_NONE
	};

	constexpr U8 MaxNumOfShaderStages = static_cast<U8>(SST_COMPUTE) + 1;



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


	constexpr U8 TotalNumberOfInternalConstantBuffers = static_cast<U8>(IUBR_LAST_USED_SLOT) + 1;

	constexpr U8 MaxNumberOfUserConstantBuffers = 10;


	TDE2_DECLARE_HANDLE_TYPE(TEventListenerId);

	constexpr TEventListenerId BroadcastListenersIdValue = TEventListenerId((std::numeric_limits<TEventListenerId>::max)());


	/*!
		enum class E_ENDIAN_TYPE

		\brief The enumeration contains a different types of endianness
	*/

	enum class E_ENDIAN_TYPE: U8
	{
		ET_LITTLE_ENDIAN,
		ET_BIG_ENDIAN,
		ET_NATIVE_ENDIAN	///< The following value is used to determine whether the native endianness is used for the host machine
	};


	TDE2_DECLARE_HANDLE_TYPE(TTextureSamplerId); ///< The type represents an identifier of a texture sampler object


	/*!
		enum class E_ADDRESS_MODE_TYPE

		\brief The enumeration contains all available types of addressing modes
	*/

	ENUM_META() enum class E_ADDRESS_MODE_TYPE
	{
		AMT_BORDER,
		AMT_CLAMP,
		AMT_MIRROR,
		AMT_WRAP,
	};


	enum class E_FILTER_FLAGS_MASKS : U32
	{
		FFM_FILTER_MIN_MASK = 0xFF,
		FFM_FILTER_MAG_MASK = 0xFF00,
		FFM_FILTER_MIP_MASK = 0xFF0000
	};


	/*!
		enum class E_TEXTURE_FILTER_TYPE

		\brief The enumeration contains all available types of sampler's filters
	*/

	ENUM_META() enum class E_TEXTURE_FILTER_TYPE : U8
	{
		FT_POINT,
		FT_BILINEAR,
		FT_TRILINEAR,
		FT_ANISOTROPIC,
		FT_UNUSED = 0x0
	};


	/*!
		struct TTextureSamplerDesc

		\brief The structure contains an information about a sampler object that is needed for its creation
	*/

	typedef struct TTextureSamplerDesc
	{
		E_TEXTURE_FILTER_TYPE mFilteringType = E_TEXTURE_FILTER_TYPE::FT_POINT;

		E_ADDRESS_MODE_TYPE   mUAddressMode = E_ADDRESS_MODE_TYPE::AMT_WRAP;
		E_ADDRESS_MODE_TYPE   mVAddressMode = E_ADDRESS_MODE_TYPE::AMT_WRAP;
		E_ADDRESS_MODE_TYPE   mWAddressMode = E_ADDRESS_MODE_TYPE::AMT_WRAP;

		bool                  mUseMipMaps = false;
	} TTextureSamplerDesc, *TTextureSamplerDescPtr;


	TDE2_DECLARE_HANDLE_TYPE(TFileEntryId);


	/*!
		enum E_BLEND_FACTOR_VALUE

		\brief The enumeration contains all possible values that can be used to
		specify blending factor's value
	*/

	ENUM_META()
	enum class E_BLEND_FACTOR_VALUE : U32
	{
		ZERO,
		ONE,
		SOURCE_ALPHA,
		ONE_MINUS_SOURCE_ALPHA,
		DEST_ALPHA,
		ONE_MINUS_DEST_ALPHA,
		CONSTANT_ALPHA,
		ONE_MINUS_CONSTANT_ALPHA,
		SOURCE_COLOR,
		ONE_MINUS_SOURCE_COLOR,
		DEST_COLOR,
		ONE_MINUS_DEST_COLOR,
	};


	/*!
		enum E_BLEND_OP_TYPE

		\brief The enumeration defines all available modes in which blending operation
		can be performed
	*/

	ENUM_META()
	enum class E_BLEND_OP_TYPE : U32
	{
		ADD,
		SUBT,
		REVERSED_SUBT
	};


	/*!
		struct TBlendStateDesc

		\brief The structure contains all information that is needed to set up a blending state
	*/

	typedef struct TBlendStateDesc
	{
		bool                 mIsEnabled = false;

		E_BLEND_FACTOR_VALUE mScrValue = E_BLEND_FACTOR_VALUE::ONE;
		E_BLEND_FACTOR_VALUE mDestValue;
		E_BLEND_OP_TYPE      mOpType = E_BLEND_OP_TYPE::ADD;

		E_BLEND_FACTOR_VALUE mScrAlphaValue = E_BLEND_FACTOR_VALUE::ONE;
		E_BLEND_FACTOR_VALUE mDestAlphaValue;
		E_BLEND_OP_TYPE      mAlphaOpType = E_BLEND_OP_TYPE::ADD;
	} TBlendStateDesc, *TBlendStateDescPtr;


	TDE2_DECLARE_HANDLE_TYPE(TBlendStateId);


	/*!
		enum class E_COMPARISON_FUNC

		\brief The enumeration contains all available types of operations which
		can be done during depth/stencil tests
	*/

	ENUM_META()
	enum class E_COMPARISON_FUNC
	{
		NEVER,
		LESS,
		EQUAL,
		LESS_EQUAL,
		GREATER,
		NOT_EQUAL,
		GREATER_EQUAL,
		ALWAYS
	};


	/*!
		enum class E_STENCIL_OP

		\brief The enumeration contains all the operations that can be
		performed over values that are stored within stencil buffer
	*/

	ENUM_META()
	enum class E_STENCIL_OP
	{
		KEEP,
		ZERO,
		REPLACE,
		INCR_SAT,
		DECR_SAT,
		INVERT,
		INCR,
		DECR
	};


	/*!
		struct TStencilOperationDesc

		\brief The type describe an operation that will be performed over stencil values
		during stencil test
	*/

	typedef struct TStencilOperationDesc
	{
		E_COMPARISON_FUNC mFunc = E_COMPARISON_FUNC::ALWAYS;

		E_STENCIL_OP      mPassOp = E_STENCIL_OP::KEEP;
		E_STENCIL_OP      mFailOp = E_STENCIL_OP::KEEP;
		E_STENCIL_OP      mDepthFailOp = E_STENCIL_OP::KEEP;
	} TStencilOpDesc, *TStencilOpDescPtr;


	/*!
		struct TDepthStencilStateDesc

		\brief The structure is a type that contains information about a depth-stencil state
	*/

	typedef struct TDepthStencilStateDesc
	{
		bool              mIsDepthTestEnabled = true;
		bool              mIsDepthWritingEnabled = true;

		E_COMPARISON_FUNC mDepthCmpFunc = E_COMPARISON_FUNC::LESS;

		bool              mIsStencilTestEnabled = false;

		U8                mStencilReadMaskValue = 0xFF;
		U8                mStencilWriteMaskValue = 0xFF;
		U8                mStencilRefValue = 0x0;
		
		TStencilOpDesc    mStencilFrontFaceOp;
		TStencilOpDesc    mStencilBackFaceOp;
	} TDepthStencilStateDesc, *TDepthStencilStateDescPtr;


	TDE2_DECLARE_HANDLE_TYPE(TDepthStencilStateId);


	/*!
		enum class E_CULL_MODE

		\brief The enumeration contains all possible modes of culling
	*/

	ENUM_META()
	enum class E_CULL_MODE
	{
		FRONT, BACK, NONE
	};

	/*!
		struct TRasterizerStateDesc

		\brief The type contains all settings that defines the way a geometry will be drawn
	*/

	typedef struct TRasterizerStateDesc
	{
		E_CULL_MODE mCullMode = E_CULL_MODE::BACK;

		bool        mIsWireframeModeEnabled = false;
		bool        mIsFrontCCWEnabled = false;

		F32         mDepthBias = 0.0f;
		F32         mMaxDepthBias = 1.0f;

		bool        mIsDepthClippingEnabled = true;
		bool        mIsScissorTestEnabled = false;
	} TRasterizerStateDesc, *TRasterizerStateDescPtr;


	TDE2_DECLARE_HANDLE_TYPE(TRasterizerStateId);


	/*!
		struct TVideoAdapterInfo

		\brief The structure contains all information about used video adapter
	*/

	typedef struct TVideoAdapterInfo
	{
		enum class E_VENDOR_TYPE: U8
		{
			NVIDIA,
			AMD,
			INTEL,
			UNKNOWN
		};

		E_VENDOR_TYPE mVendorType = E_VENDOR_TYPE::UNKNOWN;

		U64           mAvailableVideoMemory = 0;			///< Available video memory size in bytes
	} TVideoAdapterInfo, *TVideoAdapterInfoPtr;
}