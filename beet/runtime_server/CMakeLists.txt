cmake_minimum_required(VERSION 3.15)

##===EXE SOURCE===========//
add_executable(beet_server
        main.cpp
        )

##===EXE DEPENDENCIES=====//
target_include_directories(beet_server
        PUBLIC ${CMAKE_SOURCE_DIR}/beet/core/inc/
        PRIVATE ${CMAKE_SOURCE_DIR}/beet/core/inc/core/

        PUBLIC ${CMAKE_SOURCE_DIR}/beet/net/inc/
        PRIVATE ${CMAKE_SOURCE_DIR}/beet/net/inc/core/
        )
target_link_libraries(beet_server
        core
        net
        )

##===PROPERTIES===========//
set_target_properties(beet_server PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)
set_target_properties(beet_server PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/dist/server/bin")
set_target_properties(beet_server PROPERTIES OUTPUT_NAME beet_server)

##===DEBUG================//
if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(beet_server PUBLIC BEET_DEBUG=1)
    target_compile_definitions(beet_server PUBLIC _DEBUG=1)
else ()
    target_compile_definitions(beet_server PUBLIC NDEBUG=1)
endif ()