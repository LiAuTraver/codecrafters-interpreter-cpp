cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
include_directories(${CMAKE_SOURCE_DIR}/shared)

file(GLOB_RECURSE TEST_SHARED_SOURCES ${CMAKE_SOURCE_DIR}/shared/*.cpp)

enable_testing()
include(CTest)
include(GoogleTest)
if (${CMAKE_CXX_COMPILER} STREQUAL "MSVC")
    add_compile_options(/DGTEST_CREATE_SHARED_LIBRARY=1)
else ()
    add_compile_options(-DGTEST_CREATE_SHARED_LIBRARY=1)
endif ()

add_executable(loxo.test 
        scan.cpp
        parse.cpp
        evaluate.cpp
        interpret.cpp
        controlflow.cpp
        func.cpp
        ${TEST_SHARED_SOURCES}
)
target_link_libraries(loxo.test PRIVATE
        spdlog::spdlog
        fmt::fmt
        GTest::gtest
        GTest::gtest_main
        magic_enum::magic_enum
        driver
)
if (AC_CPP_DEBUG)
    copy_dlls_for(loxo.test)
endif ()
gtest_discover_tests(loxo.test)