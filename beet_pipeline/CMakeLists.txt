##===EXE SOURCE===========//
add_executable(beet_pipeline
        main.cpp
        inc/pipeline/font_shared.h
        inc/pipeline/font_atlas.h
        src/font_atlas.cpp
        inc/pipeline/pipeline_defines.h
        inc/pipeline/shader_compile.h
        src/shader_compile.cpp
        inc/pipeline/pipeline_cache.h
        src/pipeline_cache.cpp
        inc/pipeline/texture_compression.h src/texture_compression.cpp inc/pipeline/tinyddsloader.h)

##===LIB TARGET DIR=======//
target_include_directories(beet_pipeline
        PUBLIC inc
        PRIVATE inc/pipeline
        )

##===LIB THIRD PARTY======//
##===LIB CUTTLEFISH=======//
set(CUTTLEFISH_BUILD_S3TC ON)
set(CUTTLEFISH_BUILD_ETC OFF)
set(CUTTLEFISH_BUILD_ASTC OFF)
set(CUTTLEFISH_BUILD_PVRTC OFF)
add_subdirectory(third/Cuttlefish)

##===LIB FMT==============//
add_subdirectory(third/fmt)

##===LIB DEPENDENCIES=====//
target_link_libraries(beet_pipeline
        shared
        harfbuzz
        freetype
        fmt::fmt
        cuttlefish_lib
        )

##====SHADER DEPT=========//
##https://gist.github.com/evilactually/a0d191701cb48f157b05be7f74d79396
if (${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL AMD64)
    set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}\\Bin\\glslangValidator.exe")
else ()
    set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}\\Bin32\\glslangValidator.exe")
endif ()

## INFO: string replace "\\" with "/" for environment variables due to cmake
## not correctly parsing the $ENV path correctly on windows :^(
string(REPLACE "\\" "/" GLSL_VALIDATOR_WINDOWS_PATH_FIX ${GLSL_VALIDATOR})
target_compile_definitions(beet_pipeline PUBLIC GLSL_VALIDATOR_EXE_PATH="${GLSL_VALIDATOR_WINDOWS_PATH_FIX}")

##===PROPERTIES===========//
set_target_properties(beet_pipeline PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)
set_target_properties(beet_pipeline PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/dist/pipeline/bin")
set_target_properties(beet_pipeline PROPERTIES OUTPUT_NAME beet_pipeline)

##===DEBUG================//
target_compile_definitions(beet_pipeline PUBLIC BEET_PIPELINE=1)
target_compile_definitions(beet_pipeline PUBLIC "BEET_CMAKE_RES_DIR=\"${BEET_CMAKE_ROOT_DIR}/res/\"")
target_compile_definitions(beet_pipeline PUBLIC "BEET_CMAKE_CLIENT_RES_DIR=\"${CMAKE_BINARY_DIR}/dist/client/res/\"")

if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(beet_pipeline PUBLIC BEET_DEBUG=1)
    target_compile_definitions(beet_pipeline PUBLIC _DEBUG=1)
else ()
    target_compile_definitions(beet_pipeline PUBLIC NDEBUG=1)
endif ()