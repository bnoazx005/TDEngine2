cmake_minimum_required (VERSION 2.8)

project (TDEngine2SDK)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

set(TDENGINE2_LIBRARY_NAME "TDEngine2")

enable_testing()

add_subdirectory(TDEngine2)
add_subdirectory(SandboxGame)
add_subdirectory(tests)