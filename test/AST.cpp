#include <string>
#include <utility>
#include "test_env.hpp"

namespace {
std::pair<int, std::string> get_result(const auto &filepath) {
  ExecutionContext ec;
  ec.commands.push_back(ExecutionContext::parse);
  ec.input_files.push_back(filepath);
  auto exec = loxo_main(3, nullptr, ec);
  if (exec == 0) {
    return {exec, ec.output_stream.str()};
  }
  return {exec, ec.error_stream.str() + ec.output_stream.str()};
}
} // namespace
TEST(AST,print){
  auto [callback, str] = get_result("Z:/loxograph/examples/parse.expr2.lox");
  EXPECT_EQ(str, "true\n");
  EXPECT_EQ(callback, 0);
}

TEST(parse, error) {
  auto [callback, str] = get_result("Z:/loxograph/examples/parse.expr7.lox");
  EXPECT_EQ(str, "[line 1] Error at ')': Expect expression.\n");
  EXPECT_EQ(callback, 65);
}
