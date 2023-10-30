cmake_minimum_required (VERSION 3.8)

option(BUILD_D3D11_GCTX_PLUGIN "Build D3D11 Graphics context plugin" ON)
option(BUILD_GL3_0_GCTX_PLUGIN "Build GL3.0 Graphics context plugin" ON)
option(BUILD_VULKAN_GCTX_PLUGIN "Build Vulkan Graphics context plugin" ON)
option(BUILD_WINDOWS_INPUTCTX_PLUGIN "Build Windows input context plugin" ON)
option(BUILD_UNIX_INPUTCTX_PLUGIN "Build UNIX input context plugin" ON)
option(BUILD_IMGUI_CTX_PLUGIN "Build IMGUI context plugin" ON)
option(BUILD_FMOD_AUDIO_CTX_PLUGIN "Build FMOD audio context plugin" ON)
option(BUILD_LEVEL_STREAMING_PLUGIN "Build level streaming implementation plugin" ON)
option(BUILD_BULLET_PHYSICS_PLUGIN "Build Bullet physics plugin" ON)

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

if (BUILD_VULKAN_GCTX_PLUGIN)
	message(STATUS "VulkanGraphicsContext's compilation is enabled")

	add_subdirectory(VulkanGraphicsContext)
endif (BUILD_VULKAN_GCTX_PLUGIN)

if (UNIX)
	if (BUILD_UNIX_INPUTCTX_PLUGIN)
		message(STATUS "UnixInputContext's compilation is enabled")

		add_subdirectory(UnixInputContext)
	endif()
endif ()

if (BUILD_IMGUI_CTX_PLUGIN)
	message(STATUS "Immediate GUI context's compilation is enabled")

	add_subdirectory(ImGUIContext)
endif (BUILD_IMGUI_CTX_PLUGIN)

if (BUILD_FMOD_AUDIO_CTX_PLUGIN)
	message(STATUS "FMOD audio context's compilation is enabled")

	add_subdirectory(FmodAudioContext)
endif (BUILD_FMOD_AUDIO_CTX_PLUGIN)

if (BUILD_LEVEL_STREAMING_PLUGIN)
	message(STATUS "Level streaming plugin's compilation is enabled")

	add_subdirectory(TDE2LevelStreamingUtils)
endif (BUILD_LEVEL_STREAMING_PLUGIN)

if (BUILD_BULLET_PHYSICS_PLUGIN)
	message(STATUS "Bullet physics plugin's compilation is enabled")

	add_subdirectory(TDE2BulletPhysics)
endif (BUILD_BULLET_PHYSICS_PLUGIN)