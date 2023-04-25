##===EXE SOURCE===========//
add_executable(beet_client
        main.cpp
        )

##===EXE DEPENDENCIES=====//
include_directories(beet_client
        ${CMAKE_SOURCE_DIR}/beet/core/inc/
        ${CMAKE_SOURCE_DIR}/beet/maths/inc/
        )
target_link_libraries(beet_client
        core
        math
        )

##===PROPERTIES===========//
set_target_properties(beet_client PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)
set_target_properties(beet_client PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/dist/client/bin")
set_target_properties(beet_client PROPERTIES OUTPUT_NAME beet_client)

##===DEBUG================//
if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(beet_client PUBLIC BEET_DEBUG=1)
    target_compile_definitions(beet_client PUBLIC _DEBUG=1)
else ()
    target_compile_definitions(beet_client PUBLIC NDEBUG=1)
endif ()