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

TEST(ctrl, if1) {
  const auto path = R"(Z:\loxo\examples\ctrl.if1.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "adult\neligible for voting: true\n");
  EXPECT_EQ(callback, 0);
}

TEST(ctrl, if2) {
  const auto path = R"(Z:\loxo\examples\ctrl.if2.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "if branch\n");
  EXPECT_EQ(callback, 0);
}

TEST(ctrl, if3) {
  const auto path = R"(Z:\loxo\examples\ctrl.if3.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "eligible for voting: true\neligible for driving: full "
            "license\neligible for drinking (US): true\n");
  EXPECT_EQ(callback, 0);
}

TEST(ctrl, if4) {
  const auto path = R"(Z:\loxo\examples\ctrl.if4.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "young adult\neligible for voting: true\nfirst-time voter: "
            "unlikely\neligible for driving: full license\neligible for "
            "drinking (US): true\n");
  EXPECT_EQ(callback, 0);
}
TEST(ctrl, logical1) {
  const auto path = R"(Z:\loxo\examples\ctrl.logical1.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "baz\nbaz\nworld\nbar\nbar\n");
  EXPECT_EQ(callback, 0);
}
TEST(ctrl, logical2) {
  const auto path = R"(Z:\loxo\examples\ctrl.logical2.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "41\n41\ntrue\nfalse\nfalse\ntrue\n");
  EXPECT_EQ(callback, 0);
}
TEST(ctrl, logical3) {
  const auto path = R"(Z:\loxo\examples\ctrl.logical3.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "false\ntrue\n");
  EXPECT_EQ(callback, 0);
}
TEST(ctrl, logical4) {
  const auto path = R"(Z:\loxo\examples\ctrl.logical4.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "hello\nbaz\nbaz\nbar\n");
  EXPECT_EQ(callback, 0);
}

TEST(ctrl, while1) {
  const auto path = R"(Z:\loxo\examples\ctrl.while1.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "1\n2\n3\n");
  EXPECT_EQ(callback, 0);
}

TEST(ctrl, while2) {
  const auto path = R"(Z:\loxo\examples\ctrl.while2.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "Product of numbers 1 to 5: \n120\n");
  EXPECT_EQ(callback, 0);
}

TEST(ctrl, while3) {
  const auto path = R"(Z:\loxo\examples\ctrl.while3.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "0\n1\n1\n2\n3\n5\n8\n13\n21\n34\n");
  EXPECT_EQ(callback, 0);
}

TEST(ctrl, for1) {
  const auto path = R"(Z:\loxo\examples\ctrl.for1.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "0\n1\n0\n1\n");
  EXPECT_EQ(callback, 0);
}

TEST(ctrl, for2) {
  const auto path = R"(Z:\loxo\examples\ctrl.for2.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "0\n-1\nafter\n0\n");
  EXPECT_EQ(callback, 0);
}

TEST(ctrl, error1) {
  const auto path = R"(Z:\loxo\examples\ctrl.error1.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "[line 1] Error at 'var': Expect expression.\n");
  EXPECT_EQ(callback, 65);
}

TEST(ctrl, error2) {
  const auto path = R"(Z:\loxo\examples\ctrl.error2.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "[line 1] Error at 'var': Expect expression.\n");
  EXPECT_EQ(callback, 65);
}

TEST(ctrl, error3) {
  const auto path = R"(Z:\loxo\examples\ctrl.error3.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "[line 2] Error at '{': Expect expression.\n");
  EXPECT_EQ(callback, 65);
}

TEST(ctrl, error4) {
  const auto path = R"(Z:\loxo\examples\ctrl.error4.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "[line 2] Error at '{': Expect expression.\n");
  EXPECT_EQ(callback, 65);
}
