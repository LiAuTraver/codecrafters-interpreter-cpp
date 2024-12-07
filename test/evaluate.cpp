#include <utility>
#include "test_env.hpp"

namespace {
auto get_result(const auto &filepath) {
  ExecutionContext ec;
  ec.commands.push_back(ExecutionContext::evaluate);
  ec.input_files.push_back(filepath);
  auto exec = loxo_main(3, nullptr, ec);
  return exec ? std::make_pair(exec,
                               ec.error_stream.str() + ec.output_stream.str())
              : std::make_pair(exec, ec.output_stream.str());
}
} // namespace
TEST(add, integer) {
  auto [callback, str] = get_result("Z:/loxo/examples/eval/add1.lox");
  EXPECT_EQ(str, "5\n");
  EXPECT_EQ(callback, 0);
}

TEST(compare, string) {
  auto [callback, str] = get_result("Z:/loxo/examples/eval/str2.lox");
  EXPECT_EQ(str, "true\n");
  EXPECT_EQ(callback, 0);
}

TEST(bool, true) {
  auto [callback, str] = get_result("Z:/loxo/examples/eval/true.lox");
  EXPECT_EQ(str, "true\n");
  EXPECT_EQ(callback, 0);
}

TEST(compare, strnum) {
  auto [callback, str] = get_result("Z:/loxo/examples/eval/strnum.lox");
  EXPECT_EQ(str, "false\n");
  EXPECT_EQ(callback, 0);
}

TEST(error, unarystr) {
  auto [callback, str] = get_result("Z:/loxo/examples/eval/unarystr.lox");
  EXPECT_EQ(str, "Operand must be a number.\n[line 1]\n");
  EXPECT_EQ(callback, 70);
}

TEST(math, intermediate) {
  auto [callback, str] = get_result("Z:/loxo/examples/eval/cal.lox");
  EXPECT_EQ(str, "75\n");
  EXPECT_EQ(callback, 0);
}
