cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(gtest CONFIG REQUIRED)
find_package(benchmark CONFIG REQUIRED)

add_executable(fib.benchmark
    fib.bm.cpp
    ../shared/lox_driver.cpp
)

target_include_directories(fib.benchmark PUBLIC
    ../shared
)

target_link_libraries(fib.benchmark PUBLIC
    driver
    fmt::fmt
    spdlog::spdlog
    benchmark::benchmark
)

if(CMAKE_CXX_COMPILER_ID MATCHES MSVC)
  list(REMOVE_ITEM CMAKE_CXX_FLAGS_RELEASE "/O0")
  list(REMOVE_ITEM CMAKE_CXX_FLAGS_RELEASE "/Od")
  list(REMOVE_ITEM CMAEK_CXX_FLAGS_RELEASE "/RTC1")
  list(REMOVE_ITEM CMAEK_CXX_FLAGS_RELEASE "/D_DEBUG")
  list(REMOVE_ITEM CMAKE_CXX_FLAGS_RELEASE "/DDEBUG")
  list(REMOVE_ITEM CMAKE_CXX_FLAGS_RELEASE "/D__DEBUG__")
  list(REMOVE_ITEM CMAKE_CXX_FLAGS_RELEASE "/DAC_CPP_DEBUG")
  list(APPEND CMAKE_CXX_FLAGS_RELEASE "   /O2")
else()
  list(REMOVE_ITEM CMAKE_CXX_FLAGS_RELEASE "-O0")
  list(REMOVE_ITEM CMAKE_CXX_FLAGS_RELEASE "-Og")
  list(REMOVE_ITEM CMAKE_CXX_FLAGS_RELEASE "-D_DEBUG")
  list(REMOVE_ITEM CMAKE_CXX_FLAGS_RELEASE "-DDEBUG")
  list(REMOVE_ITEM CMAKE_CXX_FLAGS_RELEASE "-D__DEBUG__")
  list(REMOVE_ITEM CMAKE_CXX_FLAGS_RELEASE "-DAC_CPP_DEBUG")
  list(APPEND CMAKE_CXX_FLAGS_RELEASE "   -O3")
endif()

add_folder(Benchmarks)