cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

add_executable(demo
  demo_playground.cpp
)

target_link_libraries(demo PUBLIC
  fmt::fmt
  spdlog::spdlog
)
add_folder(Demos)