cmake_minimum_required(VERSION 3.13)
project(loxograph VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT DEFINED CMAKE_MAKE_PROGRAM)
    set(CMAKE_MAKE_PROGRAM "make")
endif ()

if (NOT DEFINED CMAKE_CXX_COMPILER)
    set(CMAKE_CXX_COMPILER "g++")
endif ()

if (NOT DEFINED CMAKE_C_COMPILER)
    set(CMAKE_C_COMPILER "gcc")
endif ()


option(AC_CPP_DEBUG "set the environment variable AC_CPP_DEBUG to enable debug mode" OFF)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.hpp)

add_executable(interpreter
        ${SOURCE_FILES}
)

if (DEFINED ENV{AC_CPP_DEBUG})
    option(USE_BOOST_CONTRACT "set the environment variable LOXOGRAPH_USE_BOOST_CONTRACT to enable boost contract" OFF)

    # if env AC_CPP_DEBUG is set to ON, enable testing
    if ($ENV{AC_CPP_DEBUG} STREQUAL "ON")
        message(STATUS "Debug mode is ON. Corresponding macro features will be enabled: DEBUG, _DEBUG, DEBUG_, _DEBUG_, AC_CPP_DEBUG")
        set(AC_CPP_DEBUG ON)
        find_package(fmt CONFIG REQUIRED)
        find_package(spdlog CONFIG REQUIRED)
        find_package(magic_enum CONFIG REQUIRED)
        set(SPDLOG_FMT_EXTERNAL ON)
        add_subdirectory(test)
        add_subdirectory(example)
        add_subdirectory(benchmark)
        target_compile_options(interpreter PRIVATE
                $<$<CXX_COMPILER_ID:MSVC>:/EHsc /D_DEBUG /DDEBUG /D__DEBUG__ /DAC_CPP_DEBUG>
                $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>: -D_DEBUG -DDEBUG -D__DEBUG__ -DAC_CPP_DEBUG>
        )
        target_link_libraries(interpreter PUBLIC
                fmt::fmt
                spdlog::spdlog
                magic_enum::magic_enum
        )
    endif ($ENV{AC_CPP_DEBUG} STREQUAL "ON")

    if (USE_BOOST_CONTRACT)
        find_package(boost COMPONENTS contract REQUIRED)
        message(STATUS "Boost contract is ON. Corresponding macro features will be enabled: LOXOGRAPH_USE_BOOST_CONTRACT")
        target_compile_options(interpreter
                PRIVATE
                $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>: -DLOXOGRAPH_USE_BOOST_CONTRACT>
        )
    endif (USE_BOOST_CONTRACT)
endif (DEFINED ENV{AC_CPP_DEBUG})
