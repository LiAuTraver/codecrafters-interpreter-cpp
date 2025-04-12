cmake_minimum_required(VERSION 3.21...3.31 FATAL_ERROR)
project(loxo VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 23)

# set(CMAKE_CXX_STANDARD_REQUIRED ON)
# disable those additionial targets added by CTest
# see https://stackoverflow.com/questions/56089330/cmake-creates-lots-of-targets-i-didnt-specify
set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)

include(cmake/prelude.cmake)
include(cmake/folder.cmake)

# for codecrafters' test
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux" AND NOT DEFINED ENV{AC_CPP_DEBUG})
  set(CMAKE_CXX_COMPILER "g++")
  set(CMAKE_MAKE_PROGRAM "make")
  set(CMAKE_C_COMPILER "gcc")
  add_compile_options(-Wno-deprecated -Wno-deprecated-declarations -Wno-c++26-extensions)
endif()

include_directories(driver/include)
include_directories(packages)
include_directories(shared)

option(AC_CPP_DEBUG "set the environment variable AC_CPP_DEBUG to enable debug mode" OFF)
option(LOXO_BUILD_SHARED "build shared library" OFF)

if(DEFINED ENV{AC_CPP_DEBUG})
  if($ENV{AC_CPP_DEBUG} STREQUAL "ON")
    message(STATUS "Debug mode is ON. Corresponding macro features will be enabled: DEBUG, _DEBUG, DEBUG_, _DEBUG_, AC_CPP_DEBUG")
    set(AC_CPP_DEBUG ON)
    set(VCPKG_MANIFEST_MODE OFF)
    set(VCPKG_MANIFEST_INSTALL OFF)
    set(LOXO_BUILD_SHARED ON)
  endif()
endif()

# `add_compile_options` MUST be placed before `add_library` or `add_executable`;
# in other words, it has only had effect on the targets that are created after it.
if(AC_CPP_DEBUG)
  # MUST include at the TOP level CMakelists.txt otherwise Ctest will not work!!!
  # Why cmake makes it so hard to use?
  enable_testing()
  include(CTest)

  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    add_compile_options(-D_DEBUG -DDEBUG -D__DEBUG__ -DAC_CPP_DEBUG -Wimplicit-fallthrough -Wno-deprecated-declarations -Wno-deprecated -Wno-c++26-extensions

      # MinGW does can't get through the `source_location` in file `spdlog/logger.h:334`
      -DSPDLOG_NO_EXCEPTIONS -DSPDLOG_NO_SOURCE_LOC
    )
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-D_DEBUG -DDEBUG -D__DEBUG__ -DAC_CPP_DEBUG -Wimplicit-fallthrough -Wno-deprecated-declarations -Wno-deprecated -Wno-c++26-extensions)
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options(/EHsc /D_DEBUG /DDEBUG /D__DEBUG__ /DAC_CPP_DEBUG /wd4716 /wd4530 /wd4244)
    add_link_options(/DEBUG:FULL)
  endif()

  message(STATUS "CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")

  add_subdirectory(shared/auxilia)
  find_package(auxilia QUIET)
else()
  add_compile_options(-Wno-attributes)
  include(ExternalProject)

  # Set where to place the headers
  set(AUXILIA_INCLUDE_DIR ${CMAKE_BINARY_DIR}/auxilia-src/include)

  # Download but don't build
  ExternalProject_Add(auxilia_download
    GIT_REPOSITORY https://github.com/LiAuTraver/auxilia.git
    GIT_TAG main
    SOURCE_DIR ${CMAKE_BINARY_DIR}/auxilia-src
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
  )

  # Create interface library
  add_library(auxilia INTERFACE)
  add_library(auxilia::auxilia ALIAS auxilia)
  target_include_directories(auxilia INTERFACE ${AUXILIA_INCLUDE_DIR})
  target_compile_features(auxilia INTERFACE cxx_std_23)

  # Make sure library depends on download completion
  add_dependencies(auxilia auxilia_download)
endif()

file(GLOB_RECURSE DRIVER_HEADER_FILES driver/include/*.hpp)
file(GLOB_RECURSE DRIVER_SOURCE_FILES driver/src/*.cpp)

add_library(driver OBJECT
  ${DRIVER_HEADER_FILES}
  ${DRIVER_SOURCE_FILES}
)

set_target_properties(driver PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(driver PUBLIC
  auxilia::auxilia
)

add_executable(interpreter
  tools/lox_interpreter.cpp
  shared/loxo_driver.cpp
  shared/execution_context.hpp
)

target_link_libraries(interpreter PUBLIC
  driver
)

if(AC_CPP_DEBUG)
  include(cmake/debug_mode.cmake)
  add_subdirectory(test)
  add_subdirectory(benchmark)
  add_subdirectory(demo)
endif()

add_folder(Tools)
