
cmake_minimum_required(VERSION 3.23)
project(beetroot)

##===SOURCE===============//

file(GLOB_RECURSE BEET_CORE_SOURCES
        core/src/*.cpp
        core/inc/**/*.h
        )

add_executable(beet
        ${BEET_CORE_SOURCES}
        )

include_directories(beet
        core/inc/
        )

##===PROPERTIES===========//

set_target_properties(beet PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)
set_target_properties(beet PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/dist/bin")
set_target_properties(beet PROPERTIES OUTPUT_NAME beet_build)

##===DEBUG================//

if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(beet PUBLIC BEET_DEBUG=1)
    target_compile_definitions(beet PUBLIC _DEBUG=1)
else ()
    target_compile_definitions(beet PUBLIC NDEBUG=1)
endif ()