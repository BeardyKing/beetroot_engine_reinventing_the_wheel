##===LIB SOURCE===========//
add_library(core
        inc/core/defines.h
        inc/core/input.h
        inc/core/input_types.h
        inc/core/time.h
        inc/core/window.h
        src/input.cpp
        src/time.cpp
        src/window.cpp
        inc/core/engine.h
        src/engine.cpp
        )

##===LIB TARGET DIR=======//
target_include_directories(core
        PUBLIC inc
        PRIVATE inc/core

        PUBLIC ${CMAKE_SOURCE_DIR}/beet/net/inc/
        PRIVATE ${CMAKE_SOURCE_DIR}/beet/net/inc/core/
        )

##===LIB DEPENDENCIES=====//
target_link_libraries(core
        shared
        math
        gfx
        net
        )

##===LIB INSTALL RULES====//
install(TARGETS core
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        )

set_target_properties(core PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)

##===DEBUG================//
if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(core PUBLIC BEET_DEBUG=1)
    target_compile_definitions(core PUBLIC _DEBUG=1)
else ()
    target_compile_definitions(core PUBLIC NDEBUG=1)
endif ()