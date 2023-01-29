cmake_minimum_required (VERSION 3.8)

add_subdirectory(Box2D)
add_subdirectory(backward-cpp)
add_subdirectory(marl)

if (TDE2_EDITORS_ENABLED)
	add_subdirectory(clip-1.5)
endif ()