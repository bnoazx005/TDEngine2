cmake_minimum_required (VERSION 3.8)

project (TDE2BulletPhysics CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/$<CONFIGURATION>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/$<CONFIGURATION>")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (NOT DEFINED ${TDENGINE2_LIBRARY_NAME})
	set(TDENGINE2_LIBRARY_NAME "TDEngine2")
endif ()

set(TDE2_BULLET_PHYSICS_LIBRARY_NAME "TDE2BulletPhysics")


set(TDE2_BULLET_PHYSICS_HEADERS
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CBulletPhysicsPlugin.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/ICollisionObject3D.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CBaseCollisionObject3D.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CBoxCollisionObject3D.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/ICollisionObjects3DVisitor.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CSphereCollisionObject3D.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CConvexHullCollisionObject3D.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CCapsuleCollisionObject3D.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CTrigger3D.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/ComponentsInspectors.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/CPhysics3DSystem.h"
	)

set(TDE2_BULLET_PHYSICS_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CBulletPhysicsPlugin.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CBaseCollisionObject3D.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CBoxCollisionObject3D.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CSphereCollisionObject3D.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CConvexHullCollisionObject3D.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CCapsuleCollisionObject3D.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CTrigger3D.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/ComponentsInspectors.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/source/CPhysics3DSystem.cpp"
	)

source_group("includes" FILES ${TDE2_BULLET_PHYSICS_HEADERS})
source_group("sources" FILES ${TDE2_BULLET_PHYSICS_SOURCES})


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

add_library(${TDE2_BULLET_PHYSICS_LIBRARY_NAME} SHARED ${TDE2_BULLET_PHYSICS_SOURCES} ${TDE2_BULLET_PHYSICS_HEADERS})

#link Bullet3 library
if (USE_EXTERNAL_BULLET_LIBRARY)	
	message(STATUS "Bullet3 library's found")

    find_package(Bullet REQUIRED)
else ()
	# use Bullet3 from deps/ directory
	message(STATUS "Builtin bullet3 library is being used...")

	set (BULLET_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/bullet/src/")
	set (BULLET_LIBRARIES 
		"${CMAKE_CURRENT_SOURCE_DIR}/deps/bullet3/lib/$<CONFIGURATION>/${CMAKE_STATIC_LIBRARY_PREFIX}BulletDynamics${CMAKE_STATIC_LIBRARY_SUFFIX}"
		"${CMAKE_CURRENT_SOURCE_DIR}/deps/bullet3/lib/$<CONFIGURATION>/${CMAKE_STATIC_LIBRARY_PREFIX}BulletCollision${CMAKE_STATIC_LIBRARY_SUFFIX}"
		"${CMAKE_CURRENT_SOURCE_DIR}/deps/bullet3/lib/$<CONFIGURATION>/${CMAKE_STATIC_LIBRARY_PREFIX}LinearMath${CMAKE_STATIC_LIBRARY_SUFFIX}")
endif ()

target_include_directories(${TDE2_BULLET_PHYSICS_LIBRARY_NAME} PUBLIC ${BULLET_INCLUDE_DIR})
target_link_libraries(${TDE2_BULLET_PHYSICS_LIBRARY_NAME} PUBLIC ${BULLET_LIBRARIES})

SET_TARGET_PROPERTIES(${TDE2_BULLET_PHYSICS_LIBRARY_NAME} PROPERTIES PREFIX "")

#set up TDEngine2 headers
target_include_directories(${TDE2_BULLET_PHYSICS_LIBRARY_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../include")

# disable the <libname>_EXPORTS
set_target_properties(${TDE2_BULLET_PHYSICS_LIBRARY_NAME} PROPERTIES DEFINE_SYMBOL "")

# bad decision to create a circular dependency TODO: dettach the plugin from TDEngine2
target_link_libraries(${TDE2_BULLET_PHYSICS_LIBRARY_NAME} PUBLIC ${TDENGINE2_LIBRARY_NAME})