#include <string>
#include <utility>
#include <gtest/gtest.h>
#include "test_env.hpp"

namespace {
auto get_result(const auto &filepath) {
  ExecutionContext ec;
  ec.commands.emplace_back(ExecutionContext::parse);
  ec.input_files.emplace_back(filepath);
  auto exec = loxo_main(3, nullptr, ec);
  return exec ? std::make_pair(exec,
                               ec.error_stream.str() + ec.output_stream.str())
              : std::make_pair(exec, ec.output_stream.str());
}
} // namespace
TEST(parse, print) {
  auto [callback, str] = get_result(LOXO_ROOT_DIR "/examples/parsing/true.lox");
  EXPECT_EQ(str, "true\n");
  EXPECT_EQ(callback, 0);
}

TEST(parse, error) {
  auto [callback, str] =
      get_result(LOXO_ROOT_DIR "/examples/parsing/error.lox");
  EXPECT_EQ(str, "[line 1] Error at ')': Expect expression.\n");
  EXPECT_EQ(callback, 65);
}
