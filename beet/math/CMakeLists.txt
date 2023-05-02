##===LIB SOURCE===========//
add_library(math
        inc/math/vec2.h
        src/vec2.cpp
        )

##===LIB TARGET DIR=======//
target_include_directories(math
        PUBLIC inc
        PRIVATE inc/math
        )

##===LIB INSTALL RULES====//
install(TARGETS math
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        )

##===DEBUG================//
if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(math PUBLIC BEET_DEBUG=1)
    target_compile_definitions(math PUBLIC _DEBUG=1)
else ()
    target_compile_definitions(math PUBLIC NDEBUG=1)
endif ()