cmake_minimum_required (VERSION 3.8)

project (Box2D CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin/$<CONFIGURATION>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin/$<CONFIGURATION>")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(BOX2D_LIBRARY_NAME "Box2D")

set(MAIN_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/Box2D.h")

set(COLLISION
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Collision/Shapes/b2ChainShape.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Collision/Shapes/b2ChainShape.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Collision/Shapes/b2CircleShape.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Collision/Shapes/b2CircleShape.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Collision/Shapes/b2EdgeShape.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Collision/Shapes/b2EdgeShape.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Collision/Shapes/b2PolygonShape.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Collision/Shapes/b2PolygonShape.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Collision/Shapes/b2Shape.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Collision/b2BroadPhase.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Collision/b2BroadPhase.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Collision/b2CollideCircle.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Collision/b2CollideEdge.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Collision/b2CollidePolygon.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Collision/b2Collision.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Collision/b2Collision.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Collision/b2Distance.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Collision/b2Distance.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Collision/b2DynamicTree.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Collision/b2DynamicTree.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Collision/b2TimeOfImpact.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Collision/b2TimeOfImpact.cpp")

set(COMMON
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Common/b2BlockAllocator.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Common/b2BlockAllocator.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Common/b2Draw.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Common/b2Draw.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Common/b2GrowableStack.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Common/b2Math.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Common/b2Math.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Common/b2Settings.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Common/b2Settings.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Common/b2StackAllocator.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Common/b2StackAllocator.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Common/b2Timer.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Common/b2Timer.cpp")

set(DYNAMICS
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/Contacts/b2ChainAndCircleContact.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/Contacts/b2ChainAndCircleContact.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/Contacts/b2ChainAndPolygonContact.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/Contacts/b2ChainAndPolygonContact.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/Contacts/b2CircleContact.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/Contacts/b2CircleContact.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/Contacts/b2Contact.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/Contacts/b2Contact.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/Contacts/b2ContactSolver.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/Contacts/b2ContactSolver.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/Contacts/b2EdgeAndCircleContact.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/Contacts/b2EdgeAndCircleContact.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/Contacts/b2EdgeAndPolygonContact.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/Contacts/b2EdgeAndPolygonContact.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/Contacts/b2PolygonAndCircleContact.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/Contacts/b2PolygonAndCircleContact.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/Contacts/b2PolygonContact.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/Contacts/b2PolygonContact.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/Joints/b2DistanceJoint.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/Joints/b2DistanceJoint.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/Joints/b2FrictionJoint.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/Joints/b2FrictionJoint.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/Joints/b2GearJoint.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/Joints/b2GearJoint.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/Joints/b2Joint.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/Joints/b2Joint.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/Joints/b2MotorJoint.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/Joints/b2MotorJoint.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/Joints/b2MouseJoint.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/Joints/b2MouseJoint.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/Joints/b2PrismaticJoint.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/Joints/b2PrismaticJoint.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/Joints/b2PulleyJoint.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/Joints/b2PulleyJoint.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/Joints/b2RevoluteJoint.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/Joints/b2RevoluteJoint.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/Joints/b2RopeJoint.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/Joints/b2RopeJoint.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/Joints/b2WeldJoint.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/Joints/b2WeldJoint.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/Joints/b2WheelJoint.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/Joints/b2WheelJoint.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/b2Body.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/b2Body.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/b2ContactManager.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/b2ContactManager.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/b2Fixture.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/b2Fixture.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/b2Island.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/b2Island.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/b2TimeStep.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/b2World.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/b2World.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/b2WorldCallbacks.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Dynamics/b2WorldCallbacks.cpp")

set(ROPE 
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Rope/b2Rope.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/Box2D/Rope/b2Rope.cpp")
	
source_group("" FILES ${MAIN_HEADER})
source_group("Collision" FILES ${COLLISION})
source_group("Common" FILES ${COMMON})
source_group("Dynamics" FILES ${DYNAMICS})
source_group("Rope" FILES ${ROPE})

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


add_library(${BOX2D_LIBRARY_NAME} STATIC ${MAIN_HEADER} ${COLLISION} ${DYNAMICS} ${COMMON} ${ROPE})

target_include_directories(${BOX2D_LIBRARY_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/")

# disable the <libname>_EXPORTS
set_target_properties(${BOX2D_LIBRARY_NAME} PROPERTIES DEFINE_SYMBOL "")