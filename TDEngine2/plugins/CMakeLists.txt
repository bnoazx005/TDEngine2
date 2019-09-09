cmake_minimum_required (VERSION 3.8)

option(BUILD_D3D11_GCTX_PLUGIN "Build D3D11 Graphics context plugin" ON)
option(BUILD_WINDOWS_INPUTCTX_PLUGIN "Build Windows input context plugin" ON)
option(BUILD_GL3_0_GCTX_PLUGIN "Build GL3.0 Graphics context plugin" ON)
option(BUILD_UNIX_INPUTCTX_PLUGIN "Build UNIX input context plugin" ON)
option(BUILD_YAML_SUPPORT_PLUGIN "Build YAML format support plugin" ON)

if (BUILD_D3D11_GCTX_PLUGIN)
	if (MSVC)
		message(STATUS "D3D11GraphicsContext's compilation is enabled")

		add_subdirectory(D3D11GraphicsContext)

		message(STATUS "WindowsInputContext's compilation is enabled")

		add_subdirectory(WindowsInputContext)
	else()
		message(STATUS "D3D11GraphicsContext's compilation is disabled")

		message(STATUS "WindowsInputContext's compilation is disabled")
	endif (MSVC)
endif (BUILD_D3D11_GCTX_PLUGIN)

if (BUILD_GL3_0_GCTX_PLUGIN)
	message(STATUS "GLGraphicsContext's compilation is enabled")

	add_subdirectory(OGLGraphicsContext)
endif (BUILD_GL3_0_GCTX_PLUGIN)

if (UNIX)
	if (BUILD_UNIX_INPUTCTX_PLUGIN)
		message(STATUS "UnixInputContext's compilation is enabled")

		add_subdirectory(UnixInputContext)
	endif()
endif ()

if (BUILD_YAML_SUPPORT_PLUGIN)
	message(STATUS "YAML support plugin's compilation is enabled")
	add_subdirectory(YAMLFormatSupport)
else ()
	message(STATUS "YAML support plugin's compilation is disabled")	
endif ()