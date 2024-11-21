#include <gtest/gtest.h>
#include <filesystem>
#include <source_location>
#include <sstream>
#include <string>
#include "config.hpp"
#include "../tools/execution_context.hpp"
static constexpr auto you_should_not_include_this_header_file_twice =
    __FILE__ "<-- this_file_is_left_without_include_gurad_by_design -->";

namespace net::ancillarycat::loxograph {
struct ExecutionContext;
nodiscard_msg(loxo_main) extern int loxo_main(int, char **, ExecutionContext &);
} // namespace net::ancillarycat::loxograph
using namespace net::ancillarycat::loxograph;
using namespace std::string_view_literals;
using std::filesystem::path;
