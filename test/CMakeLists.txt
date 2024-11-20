cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(gtest CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
include_directories(${CMAKE_SOURCE_DIR}/include)

include(GoogleTest)
# enable_testing()
if(AC_CPP_DEBUG)
  macro(copy_dlls_for target)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMENT "copy necessary dynamic libraries to the test directory"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:driver>
        $<TARGET_FILE_DIR:${target}>
    )
  endmacro(copy_dlls_for)
endif()
add_executable(spdlog_test
    spdlog.cpp
)
target_link_libraries(spdlog_test PRIVATE
    spdlog::spdlog
    fmt::fmt
    GTest::gtest
    GTest::gtest_main
    magic_enum::magic_enum
    driver
)

# gtest_discover_tests(spdlog_test)
# or:
add_test(
    NAME spdlog_test
    COMMAND spdlog_test
)

add_executable(scan_test
    scan.cpp
    ${INTERPRETER_SOURCE_FILES}
)

target_link_libraries(scan_test PRIVATE
    spdlog::spdlog
    fmt::fmt
    GTest::gtest
    GTest::gtest_main
    magic_enum::magic_enum
    driver
)
copy_dlls_for(scan_test)
gtest_discover_tests(scan_test)
