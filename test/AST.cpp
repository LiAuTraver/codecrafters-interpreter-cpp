#include "test_env.hpp"

auto get_result(const auto &filepath) {
  ExecutionContext ec;
  ec.commands.push_back(ExecutionContext::parse);
  ec.input_files.push_back(filepath);
  auto _ = loxo_main(3, nullptr, ec);
  return ec.output_stream.str();
}

TEST(AST, dummy) {
  auto result = get_result("Z:/loxograph/examples/parse.expr2.lox");
  EXPECT_EQ(result, "true\n");
}
