cmake_minimum_required (VERSION 2.8)

project (tests CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

set(EXECUTABLE_NAME "tests")

if (NOT DEFINED ${TDENGINE2_LIBRARY_NAME})
	set(TDENGINE2_LIBRARY_NAME "TDEngine2")
endif ()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# attach TDEngine2
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../TDEngine2/include")
link_directories("${CMAKE_CURRENT_SOURCE_DIR}/../bin")

# include Catch2
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/lib/Catch2/contrib/")
add_subdirectory(lib/Catch2)

set(HEADERS )

set(SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/graphics/CGraphicsLayersInfoTests.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/utils/CResultTests.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/utils/UtilsTests.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/platform/CTimerTests.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")

source_group("includes" FILES ${HEADERS})
source_group("sources" FILES ${SOURCES})

if (MSVC) 	#cl.exe compiler's options

	#Debug compiler's options
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /std:c++17 /MDd /W3 /GS /Zc:inline /Od /Gm /ZI  /Zc:wchar_t")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /RTC1 /Gd /Oy- /EHsc /nologo /diagnostics:classic /errorReport:prompt /sdl /permissive- /analyze-")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D _DEBUG")

	#Release compiler's options
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /std:c++17 /permissive- /GS /GL /analyze- /W3 /Gy /Zc:wchar_t /Zi /Gm- /O2 /sdl /Zc:inline")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /fp:precise /D _WINDLL /D _MBCS /errorReport:prompt /WX- /Zc:forScope /Gd /Oy- /Oi /MD /EHsc /nologo /diagnostics:classic")

endif(MSVC)


if (UNIX)
	message(STATUS "UNIX system has detected...")

	include(CheckCXXCompilerFlag)

	CHECK_CXX_COMPILER_FLAG("-std=c++1y" COMPILER_SUPPORTS_CXX14)

	if(COMPILER_SUPPORTS_CXX14)
		message(STATUS "C++14 is enabled")

		set(CMAKE_CXX_STANDARD 14)
		set(CMAKE_CXX_STANDARD_REQUIRED ON)
	else()
		message(ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++14 support. Please use a different C++ compiler.")
	endif()
endif (UNIX)

add_executable("${EXECUTABLE_NAME}" ${SOURCES} ${HEADERS})

if (UNIX)
	set_target_properties("${EXECUTABLE_NAME}" PROPERTIES LINK_FLAGS "-Wl,-rpath,./")
endif ()

target_link_libraries("${EXECUTABLE_NAME}" ${TDENGINE2_LIBRARY_NAME})

target_link_libraries("${EXECUTABLE_NAME}" Catch2::Catch2)

include(CTest)
include(Catch)

catch_discover_tests("${EXECUTABLE_NAME}")

# copy the engine's dll into /tests subdirectory for its testing
add_custom_command(TARGET ${EXECUTABLE_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
	"$<TARGET_FILE:${TDENGINE2_LIBRARY_NAME}>"
	"$<TARGET_FILE_DIR:${EXECUTABLE_NAME}>")