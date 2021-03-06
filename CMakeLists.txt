cmake_minimum_required (VERSION 3.8)

project (TDEngine2SDK)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/$<CONFIGURATION>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/$<CONFIGURATION>")

# Global variables are declared here
set(TDENGINE2_LIBRARY_NAME "TDEngine2")
set(TDENGINE2_TESTS_NAME "tests")

# Global options are declared here
option(IS_PLUGIN_BUILDING_ENABLED "The option shows whether plugins should be built or not" ON)
option(IS_SAMPLES_BUILDING_ENABLED "The option shows whether sample projects should be built or not" ON)
option(IS_TESTING_ENABLED "The option turns on/off tests" ON)

if (IS_TESTING_ENABLED)
	enable_testing()
endif ()

add_subdirectory(TDEngine2)

if (IS_SAMPLES_BUILDING_ENABLED)
	add_subdirectory(SandboxGame)
endif ()

if (IS_TESTING_ENABLED)
	add_subdirectory(tests)
endif ()