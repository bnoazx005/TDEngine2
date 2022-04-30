cmake_minimum_required (VERSION 3.8)

project (FmodAudioContext CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/$<CONFIGURATION>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/$<CONFIGURATION>")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (NOT DEFINED ${TDENGINE2_LIBRARY_NAME})
	set(TDENGINE2_LIBRARY_NAME "TDEngine2")
endif ()

set(FMOD_AUDIO_CONTEXT_LIBRARY_NAME "FmodAudioContext")


set(FMOD_AUDIO_CTX_HEADERS
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CFmodAudioCtxPlugin.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CFmodAudioContext.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CFmodAudioClip.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CFmodAudioListenerUpdateSystem.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CFmodAudioSourcesUpdateSystem.h"
	)

set(FMOD_AUDIO_CTX_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CFmodAudioCtxPlugin.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CFmodAudioContext.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CFmodAudioClip.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CFmodAudioListenerUpdateSystem.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CFmodAudioSourcesUpdateSystem.cpp"
	)

source_group("includes" FILES ${FMOD_AUDIO_CTX_HEADERS})
source_group("sources" FILES ${FMOD_AUDIO_CTX_SOURCES})


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

add_library(${FMOD_AUDIO_CONTEXT_LIBRARY_NAME} SHARED ${FMOD_AUDIO_CTX_SOURCES} ${FMOD_AUDIO_CTX_HEADERS})

SET_TARGET_PROPERTIES(${FMOD_AUDIO_CONTEXT_LIBRARY_NAME} PROPERTIES PREFIX "")

#set up TDEngine2 headers
target_include_directories(${FMOD_AUDIO_CONTEXT_LIBRARY_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../include")

# disable the <libname>_EXPORTS
set_target_properties(${FMOD_AUDIO_CONTEXT_LIBRARY_NAME} PROPERTIES DEFINE_SYMBOL "")

# bad decision to create a circular dependency TODO: dettach the plugin from TDEngine2
target_link_libraries(${FMOD_AUDIO_CONTEXT_LIBRARY_NAME} PUBLIC ${TDENGINE2_LIBRARY_NAME})


set(FMOD_INCLUDES_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/deps/fmod/includes/")
set(FMOD_LIB_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/deps/fmod/lib/")


# Set up FMOD 
if (MSVC)
	set(FMOD_CORE_LIBS 
		"${FMOD_LIB_DIRECTORY}core/x64/fmod_vc.lib"
		"${FMOD_LIB_DIRECTORY}core/x64/fmodL_vc.lib"
	)

	set(FMOD_STUDIO_LIBS 
		"${FMOD_LIB_DIRECTORY}studio/x64/fmodstudio_vc.lib"
		"${FMOD_LIB_DIRECTORY}studio/x64/fmodstudioL_vc.lib"
	)

	target_link_libraries(${FMOD_AUDIO_CONTEXT_LIBRARY_NAME} PUBLIC ${FMOD_CORE_LIBS} ${FMOD_STUDIO_LIBS})
endif()

if (UNIX)
	target_link_libraries(${FMOD_AUDIO_CONTEXT_LIBRARY_NAME} PUBLIC fmod fmodstudio)
endif()

target_include_directories(${FMOD_AUDIO_CONTEXT_LIBRARY_NAME} PUBLIC "${FMOD_INCLUDES_DIRECTORY}core/" "${FMOD_INCLUDES_DIRECTORY}studio/")