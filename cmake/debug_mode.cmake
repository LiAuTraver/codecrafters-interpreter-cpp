include_guard()

if(NOT AC_CPP_DEBUG)
  message(FATAL_ERROR "This file should only be included when `AC_CPP_DEBUG` is ON.")
endif()

find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
set(SPDLOG_FMT_EXTERNAL ON)
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

macro(copy_dlls_for target)
  add_custom_command(TARGET ${target} POST_BUILD
    COMMENT "copy necessary dynamic libraries to the test directory..."
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:driver>
    $<TARGET_FILE_DIR:${target}>
  )
endmacro(copy_dlls_for)
