cmake_minimum_required(VERSION 3.15)

find_package(Vulkan REQUIRED)

##===LIB SOURCE===========//
add_library(gfx
        inc/gfx/gfx_interface.h
        src/vulkan_device.cpp
        inc/gfx/vulkan_platform_defines.h
        inc/gfx/gfx_font.h
        src/gfx_font.cpp inc/gfx/gfx_lit.h
        src/gfx_lit.cpp
        inc/gfx/gfx_types.h
        inc/gfx/gfx_headers.h
        inc/gfx/gfx_utils.h
        src/gfx_utils.cpp
        inc/gfx/gfx_samplers.h
        src/gfx_samplers.cpp
        inc/gfx/gfx_resource_db.h
        src/gfx_resource_db.cpp
        inc/gfx/gfx_command.h
        src/gfx_command.cpp
        inc/gfx/gfx_texture.h
        src/gfx_texture.cpp
        inc/gfx/gfx_mesh.h
        src/gfx_mesh.cpp
)

##===LIB TARGET DIR=======//
target_include_directories(gfx
        PUBLIC inc
        PRIVATE inc/gfx
        )

##===LIB THIRD PARTY======//
##===LIB VMA==============//
add_subdirectory(third/VulkanMemoryAllocator)

##===LIB FREETYPE=========//
option(WITH_ZLIB "Freetype: Enable zlib " OFF)
option(WITH_BZip2 "Freetype: Enable bzip2 " OFF)
option(WITH_PNG "Freetype: Build with PNG" OFF)
option(WITH_HarfBuzz "Freetype: Enable harfbuzz" ON)
set(HarfBuzz_FIND_REQUIRED OFF)
add_subdirectory(third/freetype)

##===LIB HARFBUZZ=========//
set(FT_DISABLE_HARFBUZZ OFF)
set(CMAKE_DISABLE_FIND_PACKAGE_HarfBuzz TRUE)

add_subdirectory(third/harfbuzz)

##===LIB DEPENDENCIES=====//
target_link_libraries(gfx
        shared
        math
        Vulkan::Vulkan
        VulkanMemoryAllocator
        harfbuzz
        freetype
        )

set_target_properties(gfx PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)

##===DEBUG================//
if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(gfx PUBLIC BEET_DEBUG=1)
    target_compile_definitions(gfx PUBLIC _DEBUG=1)
else ()
    target_compile_definitions(gfx PUBLIC NDEBUG=1)
endif ()