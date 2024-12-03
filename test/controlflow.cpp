#include <gtest/gtest.h>
#include <ostream>
#include <print>
#include "test_env.hpp"

namespace {
std::pair<int, std::string> get_result(const auto &filepath) {
  ExecutionContext ec;
  ec.commands.push_back(ExecutionContext::interpret);
  ec.input_files.push_back(filepath);
  auto exec = loxo_main(3, nullptr, ec);
  if (exec == 0) {
    return {exec, ec.output_stream.str()};
  }
  return {exec, ec.output_stream.str() + ec.error_stream.str()};
}
} // namespace

TEST(ctrl, if1){
  const auto path = R"(Z:\loxograph\examples\ctrl.if1.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "adult\neligible for voting: true\n");
  EXPECT_EQ(callback, 0);
}