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
  const auto path = R"(Z:\loxograph\examples\ctrl.if1.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "adult\neligible for voting: true\n");
  EXPECT_EQ(callback, 0);
}

TEST(ctrl, if2) {
  const auto path = R"(Z:\loxograph\examples\ctrl.if2.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "if branch\n");
  EXPECT_EQ(callback, 0);
}

TEST(ctrl, if3) {
  const auto path = R"(Z:\loxograph\examples\ctrl.if3.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "eligible for voting: true\neligible for driving: full "
            "license\neligible for drinking (US): true\n");
  EXPECT_EQ(callback, 0);
}

TEST(ctrl, if4) {
  const auto path = R"(Z:\loxograph\examples\ctrl.if4.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "young adult\neligible for voting: true\nfirst-time voter: "
            "unlikely\neligible for driving: full license\neligible for "
            "drinking (US): true\n");
  EXPECT_EQ(callback, 0);
}
TEST(ctrl, logical1){
  const auto path = R"(Z:\loxograph\examples\ctrl.logical1.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "baz\nbaz\nworld\nbar\nbar\n");
  EXPECT_EQ(callback, 0);
}
TEST(ctrl, logical2){
  const auto path = R"(Z:\loxograph\examples\ctrl.logical2.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "41\n41\ntrue\nfalse\nfalse\ntrue\n");
  EXPECT_EQ(callback, 0);
}
TEST(ctrl, logical3){
  const auto path = R"(Z:\loxograph\examples\ctrl.logical3.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "false\ntrue\n");
  EXPECT_EQ(callback, 0);
}