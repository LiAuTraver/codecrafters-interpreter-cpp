cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
include_directories(${CMAKE_SOURCE_DIR}/shared)

enable_testing()
include(CTest)
include(GoogleTest)

if(${CMAKE_CXX_COMPILER} STREQUAL "MSVC")
  add_compile_options(/DGTEST_CREATE_SHARED_LIBRARY=1)
  add_compile_options(/DLOX_ROOT_DIR="${PROJECT_SOURCE_DIR}")
else()
  add_compile_options(-DGTEST_CREATE_SHARED_LIBRARY=1)
  add_compile_options(-DLOX_ROOT_DIR="${PROJECT_SOURCE_DIR}")
endif()

add_executable(lox.test
  scan.test.cpp
  parse.test.cpp
  evaluate.test.cpp
  interpret.test.cpp
  controlflow.test.cpp
  function.test.cpp
  scope.test.cpp
  class.test.cpp
  
  ${CMAKE_SOURCE_DIR}/shared/lox_driver.cpp
  ${CMAKE_SOURCE_DIR}/shared/execution_context.hpp
  ${CMAKE_SOURCE_DIR}/shared/test_env.hpp
)
target_link_libraries(lox.test PRIVATE
  spdlog::spdlog
  fmt::fmt
  GTest::gtest
  GTest::gtest_main
  magic_enum::magic_enum
  driver
)

gtest_discover_tests(lox.test)

add_folder(Tests)