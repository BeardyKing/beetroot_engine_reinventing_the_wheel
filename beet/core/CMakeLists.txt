##===LIB SOURCE===========//
add_library(core
        inc/core/assert.h
        inc/core/defines.h
        inc/core/input.h
        inc/core/input_types.h
        inc/core/time.h
        inc/core/types.h
        inc/core/window.h
        src/input.cpp
        src/time.cpp
        src/window.cpp
        )

##===LIB TARGET DIR=======//
target_include_directories(core
        PUBLIC inc
        PRIVATE inc/core
        )

##===LIB DEPENDENCIES=====//
include_directories(core
        PRIVATE ${CMAKE_SOURCE_DIR}/beet/maths/inc/
        )
target_link_libraries(core
        math
        )

##===LIB INSTALL RULES====//
install(TARGETS core
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        )