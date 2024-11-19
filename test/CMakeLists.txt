cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(gtest CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
include_directories(${CMAKE_SOURCE_DIR}/src)

include(GoogleTest)
# enable_testing()

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
gtest_discover_tests(scan_test)
