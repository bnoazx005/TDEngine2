cmake_minimum_required (VERSION 3.20)

project (benchmark CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../bin/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../bin/$<CONFIGURATION>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../bin/$<CONFIGURATION>")

set(EXECUTABLE_NAME "benchmark")

if (NOT DEFINED ${TDENGINE2_LIBRARY_NAME})
	set(TDENGINE2_LIBRARY_NAME "TDEngine2")
endif ()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# attach TDEngine2
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/TDEngine2/include")
link_directories("${CMAKE_CURRENT_SOURCE_DIR}/bin")


set(HEADERS
)

set(SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/ecs/CWorldBenchmarks.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")

source_group("includes" FILES ${HEADERS})
source_group("sources" FILES ${SOURCES})


if (MSVC) 	#cl.exe compiler's options

	#Debug compiler's options
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /std:c++17 /MDd /W3 /GS /Zc:inline /Od /ZI  /Zc:wchar_t")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /RTC1 /Gd /Oy- /EHsc /nologo /diagnostics:classic /errorReport:prompt /sdl /permissive- /analyze-  /wd4250")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D _DEBUG")

	#Release compiler's options
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /std:c++17 /permissive- /GS /GL /analyze- /W3 /Gy /Zc:wchar_t /Zi /O2 /sdl /Zc:inline  /wd4250")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /fp:precise /D _WINDLL /D _MBCS /errorReport:prompt /WX- /Zc:forScope /Gd /Oy- /Oi /MD /EHsc /nologo /diagnostics:classic")

endif(MSVC)


if (UNIX)
	message(STATUS "UNIX system has detected...")

	include(CheckCXXCompilerFlag)

	CHECK_CXX_COMPILER_FLAG("-std=c++1z" COMPILER_SUPPORTS_CXX17)

	if(COMPILER_SUPPORTS_CXX17)		
		set(CMAKE_CXX_STANDARD 17)
		set(CMAKE_CXX_STANDARD_REQUIRED ON)
	else()
		message(ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++17 support. Please use a different C++ compiler.")
	endif()
endif (UNIX)


add_executable(${EXECUTABLE_NAME} ${SOURCES} ${HEADERS})

# dependencies of TDEngine2
target_include_directories(${EXECUTABLE_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/TDEngine2/deps/glew-2.1.0/include")
target_include_directories(${EXECUTABLE_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/TDEngine2/deps/Wrench/source")
target_include_directories(${EXECUTABLE_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/TDEngine2/deps/Box2D/")
target_include_directories(${EXECUTABLE_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/TDEngine2/deps/optick/include")
target_include_directories(${EXECUTABLE_NAME} PUBLIC "deps/picobench")

if (UNIX)
	set_target_properties(${EXECUTABLE_NAME} PROPERTIES LINK_FLAGS "-Wl,-rpath,./")
endif ()

target_link_libraries(${EXECUTABLE_NAME} PUBLIC ${TDENGINE2_LIBRARY_NAME})