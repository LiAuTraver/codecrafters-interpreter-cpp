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
enable_testing()
if (AC_CPP_DEBUG)
    macro(copy_dlls_for target)
        add_custom_command(TARGET ${target} POST_BUILD
                COMMENT "copy necessary dynamic libraries to the test directory"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:driver>
                $<TARGET_FILE_DIR:${target}>
        )
    endmacro(copy_dlls_for)
endif ()

function(create_test_executable test_name)
    add_executable(${test_name} ${ARGN})
    target_link_libraries(${test_name} PRIVATE
            spdlog::spdlog
            fmt::fmt
            GTest::gtest
            GTest::gtest_main
            magic_enum::magic_enum
            driver
    )
    if (AC_CPP_DEBUG)
        copy_dlls_for(${test_name})
    endif ()
    gtest_discover_tests(${test_name})
endfunction(create_test_executable)

create_test_executable(spdlog_test spdlog.cpp)
create_test_executable(scan_test scan.cpp ${TEST_SHARED_SOURCES})
create_test_executable(parse_test parse.cpp ${TEST_SHARED_SOURCES})
create_test_executable(evaluate_test evaluate.cpp ${TEST_SHARED_SOURCES})
create_test_executable(interpret_test interpret.cpp ${TEST_SHARED_SOURCES})