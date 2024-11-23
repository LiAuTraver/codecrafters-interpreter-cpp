#include "test_env.hpp"

auto get_result(const auto &filepath) {
  ExecutionContext ec;
  ec.commands.push_back(ExecutionContext::parse);
  ec.input_files.push_back(filepath);
  if (auto exec = loxo_main(3, nullptr, ec); exec == 0) {
    return ec.output_stream.str();
  }
  return ec.error_stream.str() + ec.output_stream.str();
}

TEST(AST, dummy) {
  auto result = get_result("Z:/loxograph/examples/parse.expr2.lox");
  EXPECT_EQ(result, "true\n");
}

TEST(parse, error) {
  auto result = get_result("Z:/loxograph/examples/parse.expr7.lox");
  EXPECT_EQ(result, "[line 1] Error at ')': Expect expression.\n");
}
