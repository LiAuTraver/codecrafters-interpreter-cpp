cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(gtest CONFIG REQUIRED)

add_executable(spdlog_example spdlog.cpp)
target_link_libraries(spdlog_example PRIVATE
	spdlog::spdlog
	fmt::fmt
)
