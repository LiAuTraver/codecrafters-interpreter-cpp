cmake_minimum_required(VERSION 3.13)
project(loxograph VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# for codecrafters' test
if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux" AND NOT DEFINED ENV{AC_CPP_DEBUG})
    set(CMAKE_CXX_COMPILER "g++")
    set(CMAKE_MAKE_PROGRAM "make")
    set(CMAKE_C_COMPILER "gcc")
    add_compile_options(-Wno-deprecated -Wno-deprecated-declarations)
endif ()

file(GLOB_RECURSE DRIVER_HEADER_FILES driver/include/*.hpp driver/include/*.h)
file(GLOB_RECURSE DRIVER_SOURCE_FILES driver/src/*.cpp)
list(REMOVE_ITEM DRIVER_SOURCE_FILES tools/loxograph_driver.cpp)
include_directories(driver/include)
include_directories(packages)
include_directories(shared)

option(AC_CPP_DEBUG "set the environment variable AC_CPP_DEBUG to enable debug mode" OFF)
option(USE_BOOST_CONTRACT "set the environment variable LOXOGRAPH_USE_BOOST_CONTRACT to enable boost contract; only works when AC_CPP_DEBUG is enabled"
        OFF)
option(LOXOGRAPH_BUILD_SHARED "build shared library" OFF)

if (DEFINED ENV{AC_CPP_DEBUG})
    if ($ENV{AC_CPP_DEBUG} STREQUAL "ON")
        message(STATUS "Debug mode is ON. Corresponding macro features will be enabled: DEBUG, _DEBUG, DEBUG_, _DEBUG_, AC_CPP_DEBUG")
        set(AC_CPP_DEBUG ON)
    endif ()
endif ()

# `add_compile_options` MUST be placed before `add_library` or `add_executable`;
# in other words, it has only had effect on the targets that are created after it.
if (AC_CPP_DEBUG)
    # MUST include at the TOP level CMakelists.txt otherwise Ctest will not work!!!
    # Why cmake makes it so hard to use?
    enable_testing()
    include(CTest)

    if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
        add_compile_options(-D_DEBUG -DDEBUG -D__DEBUG__ -DAC_CPP_DEBUG -Wimplicit-fallthrough -Wno-deprecated-declarations -Wno-deprecated

                # MinGW does can't get through the `source_location` in file `spdlog/logger.h:334`
                -DSPDLOG_NO_EXCEPTIONS -DSPDLOG_NO_SOURCE_LOC
        )
    elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
        add_compile_options(-D_DEBUG -DDEBUG -D__DEBUG__ -DAC_CPP_DEBUG -Wimplicit-fallthrough -Wno-deprecated-declarations -Wno-deprecated)
    elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
        add_compile_options(/EHsc /D_DEBUG /DDEBUG /D__DEBUG__ /DAC_CPP_DEBUG)
    endif ()
endif ()

### after testing, build as shared library can reduce the compile time, which is good when debugging.
### note: for some wired reasons, the library would not be rebuilt if `.cpp` are changed, and `.hpp` are unchanged;
###			so turn off the shared library temporarily.
#if(LOXOGRAPH_BUILD_SHARED OR AC_CPP_DEBUG)
if (0)
    add_library(driver SHARED
            ${DRIVER_SOURCE_FILES}
    )
    target_compile_options(driver PUBLIC
            $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>: -DLIBLOXOGRAPH_SHARED>
            $<$<CXX_COMPILER_ID:MSVC>: /DLIBLOXOGRAPH_SHARED>
    )
    set_target_properties(driver PROPERTIES LINKER_LANGUAGE CXX)
    message(STATUS "Building shared library")
else ()
    add_library(driver STATIC
            ${DRIVER_HEADER_FILES}
            ${DRIVER_SOURCE_FILES}
    )
    set_target_properties(driver PROPERTIES LINKER_LANGUAGE CXX)
    message(STATUS "Building static library")
endif ()

add_executable(interpreter
        tools/interpreter.cpp
        shared/loxograph_driver.cpp
        shared/execution_context.hpp
)

target_link_libraries(interpreter PUBLIC
        driver
)

if (AC_CPP_DEBUG)
    add_subdirectory(test)
    add_subdirectory(demo)
    add_subdirectory(benchmark)
    include(cmake/debug_mode.cmake)
endif ()
