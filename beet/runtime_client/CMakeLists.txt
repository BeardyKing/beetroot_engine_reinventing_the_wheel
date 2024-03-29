cmake_minimum_required(VERSION 3.15)

##===EXE SOURCE===========//
add_executable(beet_client
        main.cpp
        inc/client/script_editor_camera.h
        src/script_editor_camera.cpp
        inc/client/client_entity_builder.h
        src/client_entity_builder.cpp
        )

##===LIB TARGET DIR=======//
target_include_directories(gfx
        PUBLIC inc
        PRIVATE inc/client
        )

target_link_libraries(beet_client
        core
        gfx
        net
        )

##===PROPERTIES===========//
set_target_properties(beet_client PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)
set_target_properties(beet_client PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/dist/client/bin")
set_target_properties(beet_client PROPERTIES OUTPUT_NAME beet_client)

##===DEBUG================//
target_compile_definitions(beet_client PUBLIC BEET_CLIENT=1)
if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(beet_client PUBLIC BEET_DEBUG=1)
    target_compile_definitions(beet_client PUBLIC _DEBUG=1)
else ()
    target_compile_definitions(beet_client PUBLIC NDEBUG=1)
endif ()

##===RES COPY=============//
file(COPY ${CMAKE_SOURCE_DIR}/res/ DESTINATION ${CMAKE_BINARY_DIR}/dist/client/res/)