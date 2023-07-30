cmake_minimum_required(VERSION 3.15)

##===LIB SOURCE===========//
add_library(math
        inc/math/utilities.h
        src/utilities.cpp

        inc/math/vec2.h
        inc/math/vec3.h
        inc/math/vec4.h
        inc/math/mat4.h
        inc/math/quat.h)

##===LIB GLM==============//
add_subdirectory(third/glm)

##===LIB DEPENDENCIES=====//
target_link_libraries(math
        glm
        )

##===LIB TARGET DIR=======//
target_include_directories(math
        PUBLIC inc
        PRIVATE inc/math
        )

##===DEBUG================//
if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(math PUBLIC BEET_DEBUG=1)
    target_compile_definitions(math PUBLIC _DEBUG=1)
else ()
    target_compile_definitions(math PUBLIC NDEBUG=1)
endif ()