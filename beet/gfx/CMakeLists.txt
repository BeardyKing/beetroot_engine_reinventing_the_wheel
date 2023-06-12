cmake_minimum_required(VERSION 3.21)

find_package(Vulkan REQUIRED)

##===LIB SOURCE===========//
add_library(gfx
        inc/gfx/gfx_interface.h
        src/vulkan_device.cpp
        inc/gfx/vulkan_platform_defines.h
        )

##===LIB TARGET DIR=======//
target_include_directories(gfx
        PUBLIC inc
        PRIVATE inc/gfx
        )

##===LIB THIRD PARTY======//
project(vma)
add_library(vma INTERFACE)

##===LIB DEPENDENCIES=====//
target_link_libraries(gfx
        shared
        math
        Vulkan::Vulkan
        vma
        )

##===LIB INSTALL RULES====//
install(TARGETS gfx
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        )

set_target_properties(gfx PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)

##===DEBUG================//
if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(gfx PUBLIC BEET_DEBUG=1)
    target_compile_definitions(gfx PUBLIC _DEBUG=1)
else ()
    target_compile_definitions(gfx PUBLIC NDEBUG=1)
endif ()