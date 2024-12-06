#include <gtest/gtest.h>
#include <utility>
#include "test_env.hpp"

namespace {
auto get_result(const auto &filepath) {
  ExecutionContext ec;
  ec.commands.push_back(ExecutionContext::interpret);
  ec.input_files.push_back(filepath);
  auto exec = loxo_main(3, nullptr, ec);
  return exec ? std::make_pair(exec,
                               ec.output_stream.str() + ec.error_stream.str())
              : std::make_pair(exec, ec.output_stream.str());
}
} // namespace
TEST(function, native1) {
  const auto path = R"(Z:\loxo\examples\fn\native1.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "Plenty of time remaining: \n2\nGetting closer: \n1\nCountdown "
            "complete!\n");
  EXPECT_EQ(callback, 0);
}

TEST(function, custom1) {
  const auto path = R"(Z:\loxo\examples\fn\custom1.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "2601\n");
  EXPECT_EQ(callback, 0);
}

TEST(function, return1) {
  const auto path = R"(Z:\loxo\examples\fn\return1.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "can see me\n3\ndivide by 0 is not allowed. will show 'nan'.\nnan\n");
  EXPECT_EQ(callback, 0);
}