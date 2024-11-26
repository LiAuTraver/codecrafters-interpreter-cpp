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
  return {exec, ec.error_stream.str() + ec.output_stream.str()};
}
} // namespace
TEST(add, integer) {
  auto [callback, str] = get_result("Z:/loxograph/examples/eval.add1.lox");
  EXPECT_EQ(str, "5\n");
  EXPECT_EQ(callback, 0);
}

TEST(bool, true) {
  auto [callback, str] = get_result("Z:/loxograph/examples/eval.true.lox");
  EXPECT_EQ(str, "true\n");
  EXPECT_EQ(callback, 0);
}

TEST(nil, nil) {
  auto [callback, str] = get_result("Z:/loxograph/examples/dynamic.lox");
  EXPECT_EQ(str, "Operand must be a number.\n[line 1]\n");
  EXPECT_EQ(callback, 70);
}

TEST(math, intermediate) {
  auto [callback, str] = get_result("Z:/loxograph/examples/eval.cal.lox");
  EXPECT_EQ(str, "75\n");
  EXPECT_EQ(callback, 0);
}