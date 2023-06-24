##===EXE SOURCE===========//
add_executable(beet_pipeline
        main.cpp
        inc/pipeline/font_shared.h
        inc/pipeline/font_atlas.h
        src/font_atlas.cpp
        )

##===LIB TARGET DIR=======//
target_include_directories(beet_pipeline
        PUBLIC inc
        PRIVATE inc/pipeline
        )

##===LIB THIRD PARTY======//
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