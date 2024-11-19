include_guard()

# if env AC_CPP_DEBUG is set to ON, enable testing
if($ENV{AC_CPP_DEBUG} STREQUAL "ON")
	message(STATUS "Debug mode is ON. Corresponding macro features will be enabled: DEBUG, _DEBUG, DEBUG_, _DEBUG_, AC_CPP_DEBUG")
	set(AC_CPP_DEBUG ON)
	find_package(fmt CONFIG REQUIRED)
	find_package(spdlog CONFIG REQUIRED)
	find_package(magic_enum CONFIG REQUIRED)
	set(SPDLOG_FMT_EXTERNAL ON)
	add_subdirectory(test)
	add_subdirectory(example)
	add_subdirectory(benchmark)
	target_compile_options(interpreter PRIVATE
		$<$<CXX_COMPILER_ID:MSVC>:/EHsc /D_DEBUG /DDEBUG /D__DEBUG__ /DAC_CPP_DEBUG>
		$<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>: -D_DEBUG -DDEBUG -D__DEBUG__ -DAC_CPP_DEBUG>
	)
	target_compile_options(driver PRIVATE
		$<$<CXX_COMPILER_ID:MSVC>:/EHsc /D_DEBUG /DDEBUG /D__DEBUG__ /DAC_CPP_DEBUG>
		$<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>: -D_DEBUG -DDEBUG -D__DEBUG__ -DAC_CPP_DEBUG>
	)
	target_link_libraries(driver PUBLIC
		fmt::fmt
		spdlog::spdlog
		magic_enum::magic_enum
	)
	target_link_libraries(interpreter PUBLIC
		fmt::fmt
		spdlog::spdlog
		magic_enum::magic_enum
		driver
	)
endif($ENV{AC_CPP_DEBUG} STREQUAL "ON")

if(USE_BOOST_CONTRACT)
	find_package(boost COMPONENTS contract REQUIRED)
	message(STATUS "Boost contract is ON. Corresponding macro features will be enabled: LOXOGRAPH_USE_BOOST_CONTRACT")
	target_compile_options(driver
		PRIVATE
		$<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>: -DLOXOGRAPH_USE_BOOST_CONTRACT>
	)
endif(USE_BOOST_CONTRACT)