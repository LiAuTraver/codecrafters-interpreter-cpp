// ReSharper disable all
// NOLINTBEGIN
#include <gtest/gtest.h>
#include <filesystem>

#include "details/loxo_fwd.hpp"
#include "../shared/execution_context.hpp"
static inline auto you_should_not_include_this_header_file_twice =
    []() -> std::nullptr_t {
  LOXO_INITIALIZATION(warn);
  __FILE__ "<-- this_file_is_left_without_include_gurad_by_design";
  return nullptr;
}();
namespace net::ancillarycat::loxo {
struct ExecutionContext;
NODISCARD_LOXO(loxo_main)
extern int loxo_main(int, char **, ExecutionContext &);
} // namespace net::ancillarycat::loxo
using namespace net::ancillarycat::loxo;
using namespace std::string_view_literals;
using std::filesystem::path;
// NOLINTEND
