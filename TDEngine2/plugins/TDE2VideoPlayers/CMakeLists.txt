cmake_minimum_required (VERSION 3.8)

project (TDE2VideoPlayers CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/$<CONFIGURATION>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/$<CONFIGURATION>")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (NOT DEFINED ${TDENGINE2_LIBRARY_NAME})
	set(TDENGINE2_LIBRARY_NAME "TDEngine2")
endif ()

set(TDE2_VIDEO_PLAYERS_LIBRARY_NAME "TDE2VideoPlayers")


set(TDE2_VIDEO_PLAYERS_HEADERS
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CTDE2VideoPlayersPlugin.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CVideoProcessSystem.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CUIVideoContainerComponent.h"
	)

set(TDE2_VIDEO_PLAYERS_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CTDE2VideoPlayersPlugin.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CVideoProcessSystem.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CUIVideoContainerComponent.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/deps/theoraplay/theoraplay.c"
	)

source_group("includes" FILES ${TDE2_VIDEO_PLAYERS_HEADERS})
source_group("sources" FILES ${TDE2_VIDEO_PLAYERS_SOURCES})


if (MSVC) 	#cl.exe compiler's options
	set(CMAKE_CXX_STANDARD 14)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)

	#Debug compiler's options
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /std:c++14 /MDd /W3 /GS /Zc:inline /Od /Zi  /Zc:wchar_t")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /RTC1 /Gd /Oy- /EHsc /nologo /diagnostics:classic /errorReport:prompt /sdl /permissive- /analyze-  /wd4250")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D _DEBUG")

	#Release compiler's options
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /std:c++14 /permissive- /GS /GL /analyze- /W3 /Gy /Zc:wchar_t /Zi /O2 /sdl /Zc:inline  /wd4250")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /fp:precise /D _WINDLL /D _MBCS /errorReport:prompt /WX- /Zc:forScope /Gd /Oy- /Oi /MD /EHsc /nologo /diagnostics:classic")

endif(MSVC)

add_subdirectory(deps/ogg)
add_subdirectory(deps/vorbis)


add_library(${TDE2_VIDEO_PLAYERS_LIBRARY_NAME} SHARED ${TDE2_VIDEO_PLAYERS_SOURCES} ${TDE2_VIDEO_PLAYERS_HEADERS})

SET_TARGET_PROPERTIES(${TDE2_VIDEO_PLAYERS_LIBRARY_NAME} PROPERTIES PREFIX "")

#set up TDEngine2 headers
target_include_directories(${TDE2_VIDEO_PLAYERS_LIBRARY_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../include")

# disable the <libname>_EXPORTS
set_target_properties(${TDE2_VIDEO_PLAYERS_LIBRARY_NAME} PROPERTIES DEFINE_SYMBOL "")

# bad decision to create a circular dependency TODO: dettach the plugin from TDEngine2
target_link_libraries(${TDE2_VIDEO_PLAYERS_LIBRARY_NAME} PUBLIC ${TDENGINE2_LIBRARY_NAME})


set(OGG_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/deps/ogg/include/")
set(THEORA_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/deps/theora/include/")
set(VORBIS_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/deps/vorbis/include/")

# theora building
set(THEORA_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/theora/")

add_library(theoradec OBJECT ${LIBTHEORA_DEC} "${THEORA_SRC_DIR}/lib/apiwrapper.c"
	"${THEORA_SRC_DIR}/lib/bitpack.c" "${THEORA_SRC_DIR}/lib/dequant.c"
	"${THEORA_SRC_DIR}/lib/fragment.c" "${THEORA_SRC_DIR}/lib/idct.c"
	"${THEORA_SRC_DIR}/lib/info.c" "${THEORA_SRC_DIR}/lib/internal.c"
	"${THEORA_SRC_DIR}/lib/state.c" "${THEORA_SRC_DIR}/lib/quant.c"
	"${THEORA_SRC_DIR}/lib/decapiwrapper.c" "${THEORA_SRC_DIR}/lib/decinfo.c"
	"${THEORA_SRC_DIR}/lib/decode.c" "${THEORA_SRC_DIR}/lib/huffdec.c")

target_include_directories(theoradec PRIVATE ${OGG_HEADERS} "${THEORA_SRC_DIR}/include")
if (UNIX)
	target_compile_options(theoradec PRIVATE -Wno-shift-negative-value -Wno-shift-op-parentheses -fPIC)
	target_compile_options(vorbis PRIVATE -fPIC)
endif()

target_include_directories(${TDE2_VIDEO_PLAYERS_LIBRARY_NAME} PUBLIC ${OGG_HEADERS} ${VORBIS_HEADERS} ${THEORA_HEADERS})
target_link_libraries(${TDE2_VIDEO_PLAYERS_LIBRARY_NAME} PUBLIC ogg vorbis theoradec)