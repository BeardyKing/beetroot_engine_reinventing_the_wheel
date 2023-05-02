##===LIB SOURCE===========//
add_library(shared
        inc/shared/assert.h
        src/log.cpp inc/shared/log.h)

##===LIB TARGET DIR=======//
target_include_directories(shared
        PUBLIC inc
        PRIVATE inc/shared
        )

##===LIB INSTALL RULES====//
install(TARGETS shared
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        )

##===DEBUG================//
if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(shared PUBLIC BEET_DEBUG=1)
    target_compile_definitions(shared PUBLIC _DEBUG=1)
else ()
    target_compile_definitions(shared PUBLIC NDEBUG=1)
endif ()