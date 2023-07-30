cmake_minimum_required(VERSION 3.15)

##===LIB SOURCE===========//
add_library(net
        inc/net/sockets.h
        src/sockets_windows.cpp
        inc/net/examples/sockets_examples.h
        src/examples/sockets_examples.cpp
        )

##===LIB TARGET DIR=======//
target_include_directories(net
        PUBLIC inc
        PRIVATE inc/net
        )

##===LIB DEPENDENCIES=====//
target_link_libraries(net
        shared
        )

##===LIB INSTALL RULES====//
install(TARGETS net
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        )

if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(net PRIVATE BEET_DEBUG=1)
    target_compile_definitions(net PRIVATE _DEBUG=1)
else ()
    target_compile_definitions(net PRIVATE NDEBUG=1)
endif ()