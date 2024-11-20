cmake_minimum_required(VERSION 3.13)
project(loxograph VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux" AND DEFINED ENV{AC_CPP_DEBUG})
  if($ENV{AC_CPP_DEBUG} STREQUAL "")
    # for codecrafters' test
    set(CMAKE_CXX_COMPILER "g++")
    set(CMAKE_MAKE_PROGRAM "make")
    set(CMAKE_C_COMPILER "gcc")
    add_compile_options(-Wno-deprecated)
  endif()
endif()

file(GLOB_RECURSE DRIVER_HEADER_FILES include/*.hpp include/*.h)
file(GLOB_RECURSE DRIVER_SOURCE_FILES src/*.cpp include/*.hpp include/*.h src/*.hpp)
include_directories(include)

option(AC_CPP_DEBUG "set the environment variable AC_CPP_DEBUG to enable debug mode" OFF)
option(USE_BOOST_CONTRACT "set the environment variable LOXOGRAPH_USE_BOOST_CONTRACT to enable boost contract; only works when AC_CPP_DEBUG is enabled"
    OFF)
option(LOXOGRAPH_BUILD_SHARED "build shared library" OFF)
if(DEFINED ENV{AC_CPP_DEBUG})
  if($ENV{AC_CPP_DEBUG} STREQUAL "ON")
    set(AC_CPP_DEBUG ON)
  endif()
endif()

if(AC_CPP_DEBUG OR LOXOGRAPH_BUILD_SHARED)
  add_library(std STATIC
      include/std.hh
  )
  set_target_properties(std PROPERTIES LINKER_LANGUAGE CXX)
  add_library(driver STATIC
      ${DRIVER_SOURCE_FILES}
  )
  set_target_properties(driver PROPERTIES LINKER_LANGUAGE CXX)
  target_link_libraries(driver PUBLIC std)
else()
  add_library(std SHARED
      include/std.hh
  )
  set_target_properties(std PROPERTIES LINKER_LANGUAGE CXX)
  add_library(driver SHARED
      ${DRIVER_SOURCE_FILES}
  )
  set_target_properties(driver PROPERTIES LINKER_LANGUAGE CXX)
endif()

add_executable(interpreter
    interpreter.cpp
    src/loxograph_driver.cpp
)

target_link_libraries(interpreter PUBLIC
    driver
    std
)

if(AC_CPP_DEBUG)
  include(cmake/debug_mode.cmake)
endif()
