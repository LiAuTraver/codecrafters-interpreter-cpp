#include <gtest/gtest.h>
#include <filesystem>
#include "../tools/execution_context.hpp"
#include "../include/config.hpp"
static inline auto you_should_not_include_this_header_file_twice =
    []() -> std::nullptr_t {
  LOXOGRAPH_INITIALIZATION(trace);
  __FILE__ "<-- this_file_is_left_without_include_gurad_by_design -->";
  return nullptr;
}();
namespace net::ancillarycat::loxograph {
struct ExecutionContext;
nodiscard_msg(loxo_main) extern int loxo_main(int, char **, ExecutionContext &);
} // namespace net::ancillarycat::loxograph
using namespace net::ancillarycat::loxograph;
using namespace std::string_view_literals;
using std::filesystem::path;
