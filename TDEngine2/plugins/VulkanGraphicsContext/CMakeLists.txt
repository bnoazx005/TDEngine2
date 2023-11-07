cmake_minimum_required (VERSION 3.8)

project (VulkanGraphicsContext CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/$<CONFIGURATION>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/$<CONFIGURATION>")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (NOT DEFINED ${TDENGINE2_LIBRARY_NAME})
	set(TDENGINE2_LIBRARY_NAME "TDEngine2")
endif ()

set(VULKAN_GCTX_LIBRARY_NAME "VulkanGraphicsContext")

find_package(Vulkan REQUIRED FATAL_ERROR)

# TODO: replace this later with correct approach
if (WIN32)
	if (CMAKE_BUILD_TYPE STREQUAL "Release")
		set(SHADERC_LIBRARY "$ENV{VULKAN_SDK}/Lib/shaderc_combined.lib")
	else()
		set(SHADERC_LIBRARY "$ENV{VULKAN_SDK}/Lib/shaderc_combinedd.lib")
	endif()

	message("SHADERC_LIBRARY: " ${SHADERC_LIBRARY})
endif()

if (UNIX)
	find_library(SHADERC_LIBRARY NAMES shaderc_combined REQUIRED)
	message("SHADERC_LIBRARY: " ${SHADERC_LIBRARY})
endif()

set(VULKAN_GCTX_HEADERS
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CVulkanGraphicsContext.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/IWindowSurfaceFactory.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/win32/CWin32WindowSurfaceFactory.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CVulkanGCtxPlugin.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CVulkanBuffer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CVulkanVertexBuffer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CVulkanMappings.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CVulkanIndexBuffer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CVulkanConstantBuffer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CVulkanShader.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CVulkanShaderCompiler.h"
	# "${CMAKE_CURRENT_SOURCE_DIR}/include/CVULKANTexture2D.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CVulkanVertexDeclaration.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CVulkanGraphicsObjectManager.h"
	# "${CMAKE_CURRENT_SOURCE_DIR}/include/CVULKANRenderTarget.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CVulkanUtils.h"
	# "${CMAKE_CURRENT_SOURCE_DIR}/include/CVULKANCubemapTexture.h"
	# "${CMAKE_CURRENT_SOURCE_DIR}/include/CVULKANDepthBufferTarget.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CVulkanStructuredBuffer.h"
	)

set(VULKAN_GCTX_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CVulkanGraphicsContext.cpp"	
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CVulkanGCtxPlugin.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/win32/CWin32WindowSurfaceFactory.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CVulkanBuffer.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CVulkanVertexBuffer.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CVulkanMappings.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CVulkanIndexBuffer.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CVulkanConstantBuffer.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CVulkanShader.cpp"
	# "${CMAKE_CURRENT_SOURCE_DIR}/source/CVULKANTexture2D.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CVulkanShaderCompiler.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CVulkanVertexDeclaration.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CVulkanGraphicsObjectManager.cpp"
	# "${CMAKE_CURRENT_SOURCE_DIR}/source/CVULKANRenderTarget.cpp"
	# "${CMAKE_CURRENT_SOURCE_DIR}/source/CVULKANCubemapTexture.cpp"
	# "${CMAKE_CURRENT_SOURCE_DIR}/source/CVULKANDepthBufferTarget.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CVulkanStructuredBuffer.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/deps/vma/vk_mem_alloc.cpp"
	)

source_group("includes" FILES ${VULKAN_GCTX_HEADERS})
source_group("sources" FILES ${VULKAN_GCTX_SOURCES})

if (MSVC) 	#cl.exe compiler's options
	set(CMAKE_CXX_STANDARD 17)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)

	#Debug compiler's options
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd /W3 /GS /Zc:inline /Od /Zi  /Zc:wchar_t")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /RTC1 /Gd /Oy- /EHsc /nologo /diagnostics:classic /errorReport:prompt /sdl /permissive- /analyze-  /wd4250")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D _DEBUG")

	#Release compiler's options
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /permissive- /GS /Vulkan /analyze- /W3 /Gy /Zc:wchar_t /Zi /O2 /sdl /Zc:inline  /wd4250")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /fp:precise /D _WINDLL /D _MBCS /errorReport:prompt /WX- /Zc:forScope /Gd /Oy- /Oi /MD /EHsc /nologo /diagnostics:classic")

endif(MSVC)

if (UNIX)

	message(STATUS "UNIX system has detected...")

	include(CheckCXXCompilerFlag)

	CHECK_CXX_COMPILER_FLAG("-std=c++1z" COMPILER_SUPPORTS_CXX14)

	if(COMPILER_SUPPORTS_CXX14)
		message(STATUS "C++17 is enabled")

		set(CMAKE_CXX_STANDARD 17)
		set(CMAKE_CXX_STANDARD_REQUIRED ON)
	else()
	  message(ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++14 support. Please use a different C++ compiler.")
	endif()

endif (UNIX)


add_library(${VULKAN_GCTX_LIBRARY_NAME} SHARED ${VULKAN_GCTX_SOURCES} ${VULKAN_GCTX_HEADERS})

SET_TARGET_PROPERTIES(${VULKAN_GCTX_LIBRARY_NAME} PROPERTIES PREFIX "")

#set up TDEngine2 headers
target_include_directories(${VULKAN_GCTX_LIBRARY_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../include")

#vulkan
target_include_directories(${VULKAN_GCTX_LIBRARY_NAME} PUBLIC Vulkan::Headers)
target_link_libraries(${VULKAN_GCTX_LIBRARY_NAME} PUBLIC ${SHADERC_LIBRARY})

#vma
target_include_directories(${VULKAN_GCTX_LIBRARY_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/deps/vma/")

# volk
if (WIN32)
   set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
endif()

add_subdirectory(deps/volk)
target_link_libraries(${VULKAN_GCTX_LIBRARY_NAME} PRIVATE volk)

# disable the <libname>_EXPORTS
set_target_properties(${VULKAN_GCTX_LIBRARY_NAME} PROPERTIES DEFINE_SYMBOL "")

if (UNIX)
	install (TARGETS ${VULKAN_GCTX_LIBRARY_NAME} DESTINATION "/usr/lib/TDEngine2/plugins")
endif ()

# bad decision to create a circular dependency TODO: dettach the plugin from TDEngine2
target_link_libraries(${VULKAN_GCTX_LIBRARY_NAME} PUBLIC ${TDENGINE2_LIBRARY_NAME})