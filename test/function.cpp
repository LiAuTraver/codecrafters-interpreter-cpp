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

TEST(function, resurse1) {
  const auto path = R"(Z:\loxo\examples\fn\recurse1.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "55\ntrue\n");
  EXPECT_EQ(callback, 0);
}

TEST(function, withargs) {
  const auto path = R"(Z:\loxo\examples\fn\withargs.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "foo\n");
  EXPECT_EQ(callback, 0);
}

TEST(function, nested1) {
  const auto path = R"(Z:\loxo\examples\fn\nested1.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "Numbers >= 55:\n55\n56\n57\n58\n59\nNumbers >= 10:\n10\n11\n12\n13\n14\n");
  EXPECT_EQ(callback, 0);
}

// nested2
TEST(function, nested2) {
  const auto path = R"(Z:\loxo\examples\fn\nested2.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "6\n6\n");
  EXPECT_EQ(callback, 0);
}