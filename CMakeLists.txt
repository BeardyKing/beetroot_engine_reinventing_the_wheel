cmake_minimum_required(VERSION 3.23)
project(beetroot)

##===INTERNAL LIBS========//
add_subdirectory(beet/shared)
add_subdirectory(beet/math)
add_subdirectory(beet/net)

add_subdirectory(beet/core)

##===EXES=================//
add_subdirectory(beet/runtime_client)
add_subdirectory(beet/runtime_server)
