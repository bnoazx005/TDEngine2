cmake_minimum_required (VERSION 3.8)

option(BUILD_FONT_GENERATION_TOOL "Build font generation tool" ON)

if (BUILD_FONT_GENERATION_TOOL)
	message(STATUS "Font generation tool's compilation is enabled")
	add_subdirectory(FontGenerator)
endif (BUILD_FONT_GENERATION_TOOL)