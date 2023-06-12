##===EXE SOURCE===========//
add_executable(beet_pipeline
        main.cpp
        )

##===LIB THIRD PARTY======//
##===LIB STB==============//
include_directories(beet
        third/stb/
        )

##===LIB FREETYPE=========//
option(WITH_ZLIB "Freetype: Enable zlib " OFF)
option(WITH_BZip2 "Freetype: Enable bzip2 " OFF)
option(WITH_PNG "Freetype: Build with PNG" OFF)
option(WITH_HarfBuzz "Freetype: Enable harfbuzz" ON)
set(HarfBuzz_FIND_REQUIRED OFF)
add_subdirectory(third/freetype)

##===LIB HARFBUZZ=========//
set (FT_DISABLE_HARFBUZZ OFF)
set (CMAKE_DISABLE_FIND_PACKAGE_HarfBuzz TRUE)

add_subdirectory(third/harfbuzz)
##===LIB FMT==============//
add_subdirectory(third/fmt)

##===LIB DEPENDENCIES=====//
target_link_libraries(beet_pipeline
        shared
        harfbuzz
        freetype
        fmt::fmt
        )

##===PROPERTIES===========//
set_target_properties(beet_pipeline PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)
set_target_properties(beet_pipeline PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/dist/pipeline/bin")
set_target_properties(beet_pipeline PROPERTIES OUTPUT_NAME beet_client)

##===DEBUG================//
target_compile_definitions(beet_pipeline PUBLIC BEET_PIPELINE=1)
target_compile_definitions(beet_pipeline PUBLIC "BEET_CMAKE_RES_DIR=\"${BEET_CMAKE_ROOT_DIR}/res/\"")

if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(beet_pipeline PUBLIC BEET_DEBUG=1)
    target_compile_definitions(beet_pipeline PUBLIC _DEBUG=1)
else ()
    target_compile_definitions(beet_pipeline PUBLIC NDEBUG=1)
endif ()