##===EXE SOURCE===========//
add_executable(beet_client
        main.cpp
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
file(GLOB_RECURSE BEET_RESOURCES LIST_DIRECTORIES false "${CMAKE_SOURCE_DIR}/res/*")
foreach (RESOURCE_FILE ${BEET_RESOURCES})
    get_filename_component(FILE_NAME ${RESOURCE_FILE} NAME)
    get_filename_component(PARENT_DIR ${RESOURCE_FILE} DIRECTORY)
    get_filename_component(FILE_EXT ${RESOURCE_FILE} LAST_EXT)

    string(REGEX REPLACE "^${CMAKE_SOURCE_DIR}/res" "" PARENT_DIR ${PARENT_DIR})
    set(TARGET_FILE_NAME "${CMAKE_BINARY_DIR}/dist/client/res${PARENT_DIR}/${FILE_NAME}")

    configure_file(${RESOURCE_FILE} ${TARGET_FILE_NAME} COPYONLY)
endforeach ()