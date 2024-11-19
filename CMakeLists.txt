cmake_minimum_required(VERSION 3.13)
project(loxograph VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#if(NOT DEFINED CMAKE_MAKE_PROGRAM)
#	set(CMAKE_MAKE_PROGRAM "make")
#endif()
#
#if(NOT DEFINED CMAKE_CXX_COMPILER)
#	set(CMAKE_CXX_COMPILER "g++")
#endif()
#
#if(NOT DEFINED CMAKE_C_COMPILER)
#	set(CMAKE_C_COMPILER "gcc")
#endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	# for codecrafters' test
	set(CMAKE_CXX_COMPILER "g++")
	set(CMAKE_MAKE_PROGRAM "make")
	set(CMAKE_C_COMPILER "gcc")
endif()

file(GLOB_RECURSE INTERPRETER_SOURCE_FILES src/*.cpp include/*.hpp include/*.h src/*.hpp)
include_directories(include)

option(AC_CPP_DEBUG "set the environment variable AC_CPP_DEBUG to enable debug mode" OFF)
add_library(driver STATIC
	${INTERPRETER_SOURCE_FILES}
)

add_executable(interpreter
	interpreter.cpp
	include/loxo_main.hpp
)

target_link_libraries(interpreter PUBLIC
	driver
)

if(DEFINED ENV{AC_CPP_DEBUG})
	option(USE_BOOST_CONTRACT "set the environment variable LOXOGRAPH_USE_BOOST_CONTRACT to enable boost contract" OFF)
	include(cmake/debug_mode.cmake)
endif()

#if(NOT AC_CPP_DEBUG)
#	# codecrafter submission feature macro, to enable or disable certain features
#	target_compile_options(driver
#		PRIVATE
#		$<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>: -DLOXOGRAPH_IS_CODECRAFTER>
#	)
#endif()