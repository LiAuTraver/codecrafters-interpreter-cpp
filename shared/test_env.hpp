// NOLINTBEGIN
#include <filesystem>

#include <accat/auxilia/auxilia.hpp>
#include <execution_context.hpp>

static inline auto you_should_not_include_this_header_file_twice =
    []() -> std::nullptr_t {
  AC_SPDLOG_INITIALIZATION(loxo, warn);
  __FILE__ "<-- this_file_is_left_without_include_gurad_by_design";
  return nullptr;
}();
namespace accat::loxo {
struct ExecutionContext;
[[nodiscard]]
extern int loxo_main(int, char **, ExecutionContext &);
} // namespace accat::loxo
using namespace accat::loxo;
using namespace std::string_view_literals;
using namespace std::filesystem;
// NOLINTEND
