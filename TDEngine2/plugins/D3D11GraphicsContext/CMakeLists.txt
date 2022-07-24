cmake_minimum_required (VERSION 3.8)

project (D3D11GraphicsContext CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/$<CONFIGURATION>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/$<CONFIGURATION>")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (NOT DEFINED ${TDENGINE2_LIBRARY_NAME})
	set(TDENGINE2_LIBRARY_NAME "TDEngine2")
endif ()

set(D3D11_GCTX_LIBRARY_NAME "D3D11GraphicsContext")


set(D3D11_GCTX_HEADERS
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CD3D11GraphicsContext.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CD3D11Buffer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CD3D11Mappings.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CD3D11IndexBuffer.h"	
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CD3D11GCtxPlugin.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CD3D11VertexBuffer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CD3D11ConstantBuffer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CD3D11VertexDeclaration.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CD3D11Shader.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CD3D11ShaderCompiler.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CD3D11Texture2D.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CD3D11Utils.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CD3D11GraphicsObjectManager.h"	
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CD3D11RenderTarget.h"	
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CD3D11CubemapTexture.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CD3D11DepthBufferTarget.h"		
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CD3D11StructuredBuffer.h"		
	)

set(D3D11_GCTX_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CD3D11GraphicsContext.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CD3D11Buffer.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CD3D11Mappings.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CD3D11IndexBuffer.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CD3D11GCtxPlugin.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CD3D11VertexBuffer.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CD3D11ConstantBuffer.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CD3D11VertexDeclaration.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CD3D11Shader.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CD3D11ShaderCompiler.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CD3D11Texture2D.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CD3D11GraphicsObjectManager.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CD3D11RenderTarget.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CD3D11CubemapTexture.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CD3D11DepthBufferTarget.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CD3D11StructuredBuffer.cpp"
	)

source_group("includes" FILES ${D3D11_GCTX_HEADERS})
source_group("sources" FILES ${D3D11_GCTX_SOURCES})


if (MSVC) 	#cl.exe compiler's options
	set(CMAKE_CXX_STANDARD 14)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)

	#Debug compiler's options
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd /W3 /GS /Zc:inline /Od /Zi  /Zc:wchar_t")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /RTC1 /Gd /Oy- /EHsc /nologo /diagnostics:classic /errorReport:prompt /sdl /permissive- /analyze-  /wd4250")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D _DEBUG")

	#Release compiler's options
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /permissive- /GS /GL /analyze- /W3 /Gy /Zc:wchar_t /Zi /O2 /sdl /Zc:inline  /wd4250")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /fp:precise /D _WINDLL /D _MBCS /errorReport:prompt /WX- /Zc:forScope /Gd /Oy- /Oi /MD /EHsc /nologo /diagnostics:classic")

endif(MSVC)


add_library(${D3D11_GCTX_LIBRARY_NAME} SHARED ${D3D11_GCTX_SOURCES} ${D3D11_GCTX_HEADERS})

#set up TDEngine2 headers
target_include_directories(${D3D11_GCTX_LIBRARY_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../include")

#set up GLEW headers
target_include_directories(${D3D11_GCTX_LIBRARY_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../deps/glew-2.1.0/include")

# disable the <libname>_EXPORTS
set_target_properties(${D3D11_GCTX_LIBRARY_NAME} PROPERTIES DEFINE_SYMBOL "")

# bad decision to create a circular dependency TODO: dettach the plugin from TDEngine2
target_link_libraries(${D3D11_GCTX_LIBRARY_NAME} PUBLIC ${TDENGINE2_LIBRARY_NAME})