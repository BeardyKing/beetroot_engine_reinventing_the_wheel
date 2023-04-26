cmake_minimum_required(VERSION 3.23)
project(beetroot)

##===EXES=================//
add_subdirectory(beet/client)
add_subdirectory(beet/server)

##===INTERNAL LIBS========//
add_subdirectory(beet/math)
add_subdirectory(beet/core)
