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