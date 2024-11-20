cmake_minimum_required(VERSION 3.13)
project(loxograph VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# for codecrafters' test
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux" AND NOT DEFINED ENV{AC_CPP_DEBUG})
    set(CMAKE_CXX_COMPILER "g++")
    set(CMAKE_MAKE_PROGRAM "make")
    set(CMAKE_C_COMPILER "gcc")
    add_compile_options(-Wno-deprecated -Wno-deprecated-declarations)
endif()

file(GLOB_RECURSE DRIVER_HEADER_FILES include/*.hpp include/*.h)
file(GLOB_RECURSE DRIVER_SOURCE_FILES src/*.cpp)
list(REMOVE_ITEM DRIVER_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/loxograph_driver.cpp)
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

# `add_compile_options` MUST be placed before `add_library` or `add_executable`;
# in other words, it has only had effect on the targets that are created after it.
if(AC_CPP_DEBUG)
  if(GNU)
    add_compile_options(-D_DEBUG -DDEBUG -D__DEBUG__ -DAC_CPP_DEBUG
        # MinGW does can't get through the `source_location`
        #        -DSPDLOG_NO_SOURCE_LOC
    )
  elseif(Clang)
    add_compile_options(-D_DEBUG -DDEBUG -D__DEBUG__ -DAC_CPP_DEBUG)
  elseif(MSVC)
    add_compile_options(/EHsc /D_DEBUG /DDEBUG /D__DEBUG__ /DAC_CPP_DEBUG)
  endif()
endif()

# after testing, build as shared library can reduce the compile time, which is good when debugging.
if(LOXOGRAPH_BUILD_SHARED OR AC_CPP_DEBUG)
  add_library(driver SHARED
      ${DRIVER_SOURCE_FILES}
  )
  target_compile_options(driver PUBLIC
      $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>: -Wno-deprecated -DLIBLOXOGRAPH_SHARED>
      $<$<CXX_COMPILER_ID:MSVC>: /DLIBLOXOGRAPH_SHARED>
  )
  set_target_properties(driver PROPERTIES LINKER_LANGUAGE CXX)
  message(STATUS "Building shared library")
else()
  add_library(driver STATIC
      ${DRIVER_SOURCE_FILES}
  )
  set_target_properties(driver PROPERTIES LINKER_LANGUAGE CXX)
  message(STATUS "Building static library")
endif()

target_precompile_headers(driver PUBLIC
    include/std.hh
)
target_compile_options(driver PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>: -Wno-deprecated>
)

add_executable(interpreter
    interpreter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/loxograph_driver.cpp
)
target_compile_options(interpreter PUBLIC
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>: -Wno-deprecated>
)

target_link_libraries(interpreter PUBLIC
    driver
)

if(AC_CPP_DEBUG)
  include(cmake/debug_mode.cmake)
endif()
