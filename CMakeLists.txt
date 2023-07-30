cmake_minimum_required(VERSION 3.15)
project(beetroot)

set(BEET_CMAKE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

##===INTERNAL LIBS========//
add_subdirectory(beet/shared)
add_subdirectory(beet/math)
add_subdirectory(beet/gfx)
add_subdirectory(beet/net)
add_subdirectory(beet/core)

##===OFFLINE PIPELINE=====//
add_subdirectory(beet_pipeline)

##===EXES=================//
add_subdirectory(beet/runtime_client)
add_subdirectory(beet/runtime_server)
