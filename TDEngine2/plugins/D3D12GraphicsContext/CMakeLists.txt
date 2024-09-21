cmake_minimum_required (VERSION 3.8)

project (D3D12GraphicsContext CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/$<CONFIGURATION>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/$<CONFIGURATION>")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (NOT DEFINED ${TDENGINE2_LIBRARY_NAME})
	set(TDENGINE2_LIBRARY_NAME "TDEngine2")
endif ()

set(D3D12_GCTX_LIBRARY_NAME "D3D12GraphicsContext")


set(D3D12_GCTX_HEADERS
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CD3D12GraphicsContext.h"
	#"${CMAKE_CURRENT_SOURCE_DIR}/include/CD3D12Buffer.h"
	#"${CMAKE_CURRENT_SOURCE_DIR}/include/CD3D12Mappings.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CD3D12GCtxPlugin.h"
	#"${CMAKE_CURRENT_SOURCE_DIR}/include/CD3D12VertexDeclaration.h"
	#"${CMAKE_CURRENT_SOURCE_DIR}/include/CD3D12Shader.h"
	#"${CMAKE_CURRENT_SOURCE_DIR}/include/CD3D12ShaderCompiler.h"
	#"${CMAKE_CURRENT_SOURCE_DIR}/include/CD3D12Texture.h"
	#"${CMAKE_CURRENT_SOURCE_DIR}/include/CD3D12Utils.h"
	#"${CMAKE_CURRENT_SOURCE_DIR}/include/CD3D12GraphicsObjectManager.h"	
	#"${CMAKE_CURRENT_SOURCE_DIR}/include/CD3D12RenderTarget.h"
	#"${CMAKE_CURRENT_SOURCE_DIR}/include/CD3D12DepthBufferTarget.h"
	)

set(D3D12_GCTX_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CD3D12GraphicsContext.cpp"
	#"${CMAKE_CURRENT_SOURCE_DIR}/source/CD3D12Buffer.cpp"
	#"${CMAKE_CURRENT_SOURCE_DIR}/source/CD3D12Mappings.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CD3D12GCtxPlugin.cpp"
	#"${CMAKE_CURRENT_SOURCE_DIR}/source/CD3D12VertexDeclaration.cpp"
	#"${CMAKE_CURRENT_SOURCE_DIR}/source/CD3D12Shader.cpp"
	#"${CMAKE_CURRENT_SOURCE_DIR}/source/CD3D12ShaderCompiler.cpp"
	#"${CMAKE_CURRENT_SOURCE_DIR}/source/CD3D12Texture.cpp"
	#"${CMAKE_CURRENT_SOURCE_DIR}/source/CD3D12GraphicsObjectManager.cpp"
	#"${CMAKE_CURRENT_SOURCE_DIR}/source/CD3D12RenderTarget.cpp"
	#"${CMAKE_CURRENT_SOURCE_DIR}/source/CD3D12DepthBufferTarget.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/deps/D3D12MemAlloc/D3D12MemAlloc.cpp"
	)

source_group("includes" FILES ${D3D12_GCTX_HEADERS})
source_group("sources" FILES ${D3D12_GCTX_SOURCES})

if (PROJECT_IS_TOP_LEVEL AND MSVC) 	#cl.exe compiler's options
	set(CMAKE_CXX_STANDARD 14)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)

	#Debug compiler's options
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd /W3 /GS /Zc:inline /Od /Zi  /Zc:wchar_t")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /RTC1 /Gd /Oy- /EHsc /nologo /diagnostics:classic /errorReport:prompt /sdl /permissive- /analyze-  /wd4250")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D _DEBUG")

	#Release compiler's options
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /permissive- /GS /GL /analyze- /W3 /Gy /Zc:wchar_t /Zi /O2 /sdl /Zc:inline  /wd4250")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /fp:precise /D _WINDLL /D _MBCS /errorReport:prompt /WX- /Zc:forScope /Gd /Oy- /Oi /MD /EHsc /nologo /diagnostics:classic")

endif()


add_library(${D3D12_GCTX_LIBRARY_NAME} SHARED ${D3D12_GCTX_SOURCES} ${D3D12_GCTX_HEADERS})

#set up TDEngine2 headers
target_include_directories(${D3D12_GCTX_LIBRARY_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../include")

#set up GLEW headers
target_include_directories(${D3D12_GCTX_LIBRARY_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../deps/glew-2.1.0/include")

# D3D12MemAlloc
target_include_directories(${D3D12_GCTX_LIBRARY_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/deps/D3D12MemAlloc/")

# disable the <libname>_EXPORTS
set_target_properties(${D3D12_GCTX_LIBRARY_NAME} PROPERTIES DEFINE_SYMBOL "")

# bad decision to create a circular dependency TODO: dettach the plugin from TDEngine2
target_link_libraries(${D3D12_GCTX_LIBRARY_NAME} PUBLIC ${TDENGINE2_LIBRARY_NAME})