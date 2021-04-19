cmake_minimum_required (VERSION 3.8)

option(BUILD_FONT_GENERATION_TOOL "Build font generation tool" ON)
option(BUILD_RESOURCES_PACKER_TOOL "Build resources packer tool" ON)
option(BUILD_ANIMATION_EDITOR_TOOL "Build animation editor tool" ON)
option(BUILD_PARTICLE_EDITOR_TOOL "Build particle editor tool" ON)
option(BUILD_TEXTURE_PACKER_TOOL "Build textures packer utility tool" ON)

if (BUILD_FONT_GENERATION_TOOL)
	message(STATUS "Font generation tool's compilation is enabled")
	add_subdirectory(FontGenerator)
endif (BUILD_FONT_GENERATION_TOOL)

if (BUILD_RESOURCES_PACKER_TOOL)
	message(STATUS "Resources packer tool's compilation is enabled")
	add_subdirectory(ResourcesPacker)
endif (BUILD_RESOURCES_PACKER_TOOL)

if (BUILD_ANIMATION_EDITOR_TOOL)
	message(STATUS "Animation editor tool's compilation is enabled")
	add_subdirectory(AnimationEditor)
endif (BUILD_ANIMATION_EDITOR_TOOL)

if (BUILD_PARTICLE_EDITOR_TOOL)
	message(STATUS "Particle editor tool's compilation is enabled")
	add_subdirectory(ParticleEditor)
endif (BUILD_PARTICLE_EDITOR_TOOL)

if (BUILD_TEXTURE_PACKER_TOOL)
	message(STATUS "Textures packer utlity tool's compilation is enabled")
	add_subdirectory(TexturesPacker)
endif (BUILD_TEXTURE_PACKER_TOOL)