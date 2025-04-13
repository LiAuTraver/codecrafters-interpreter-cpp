// NOLINTBEGIN
#include <filesystem>

#include <accat/auxilia/auxilia.hpp>
#include <execution_context.hpp>

#ifndef LOX_ROOT_DIR
#  define LOX_ROOT_DIR R"(Z:\lox)"
#endif

static inline auto you_should_not_include_this_header_file_twice =
    []() -> std::nullptr_t {
  AC_SPDLOG_INITIALIZATION(lox, warn);
  __FILE__ "<-- this_file_is_left_without_include_gurad_by_design";
  return nullptr;
}();

using namespace accat::lox;
using namespace std::literals;
using namespace std::filesystem;
// NOLINTEND
