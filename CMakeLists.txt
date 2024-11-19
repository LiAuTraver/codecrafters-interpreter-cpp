cmake_minimum_required(VERSION 3.13)
project(loxograph VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux" AND EXISTS ENV{AC_CPP_DEBUG})
  if($ENV{AC_CPP_DEBUG} STREQUAL "ON")
    # for codecrafters' test
    set(CMAKE_CXX_COMPILER "g++")
    set(CMAKE_MAKE_PROGRAM "make")
    set(CMAKE_C_COMPILER "gcc")
  endif()
endif()

file(GLOB_RECURSE DRIVER_SOURCE_FILES src/*.cpp include/*.hpp include/*.h src/*.hpp)
include_directories(include)

option(AC_CPP_DEBUG "set the environment variable AC_CPP_DEBUG to enable debug mode" OFF)
option(USE_BOOST_CONTRACT "set the environment variable LOXOGRAPH_USE_BOOST_CONTRACT to enable boost contract; only works when AC_CPP_DEBUG is enabled"
    OFF)

add_library(driver SHARED
    ${DRIVER_SOURCE_FILES}
)

add_executable(interpreter
    interpreter.cpp
    src/loxograph_driver.cpp
)

target_link_libraries(interpreter PUBLIC
    driver
)

if(DEFINED ENV{AC_CPP_DEBUG})
  include(cmake/debug_mode.cmake)
endif()
