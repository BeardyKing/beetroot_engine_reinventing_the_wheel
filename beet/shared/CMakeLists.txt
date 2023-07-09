##===LIB SOURCE===========//
add_library(shared
        inc/shared/assert.h
        inc/shared/bit_utils.h
        inc/shared/log.h
        src/log.cpp
        src/bit_utils.cpp
        inc/shared/db_types.h
        )

##===LIB TARGET DIR=======//
target_include_directories(shared
        PUBLIC inc
        PRIVATE inc/shared

        PUBLIC third/stb/
        )

##===LIB THIRD PARTY======//
##===LIB STB==============//
include_directories(shared
        third/stb/
        )

##===LIB DEPENDENCIES=====//
target_link_libraries(shared
        math
        )

set_target_properties(shared PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)

##===DEBUG================//
if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(shared PUBLIC BEET_DEBUG=1)
    target_compile_definitions(shared PUBLIC _DEBUG=1)
else ()
    target_compile_definitions(shared PUBLIC NDEBUG=1)
endif ()