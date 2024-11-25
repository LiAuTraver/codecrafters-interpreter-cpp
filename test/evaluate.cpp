#include "test_env.hpp"
#include <gtest/gtest.h>
namespace {
auto get_result(const auto &filepath) {
  ExecutionContext ec;
  ec.commands.push_back(ExecutionContext::interpret);
  ec.input_files.push_back(filepath);
  if (auto exec = loxo_main(3, nullptr, ec); exec == 0) {
    return ec.output_stream.str();
  }
  return ec.error_stream.str() + ec.output_stream.str();
}
} // namespace
// TEST(add, integer) {
//   auto res = get_result("Z:/loxograph/examples/eval.add1.lox");
//   EXPECT_EQ(res, "5\n");
// }
//
// TEST(bool,true){
//   auto res = get_result("Z:/loxograph/examples/eval.true.lox");
//   EXPECT_EQ(res, "true\n");
// }

TEST(nil,nil){
  auto res = get_result("Z:/loxograph/examples/dynamic.lox");
  EXPECT_EQ(res, "world quz\n");
}
