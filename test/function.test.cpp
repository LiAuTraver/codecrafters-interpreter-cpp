#include <gtest/gtest.h>
#include <utility>
#include "test_env.hpp"

namespace {
auto get_result(const auto &filepath) {
  ExecutionContext ec;
  ec.commands.emplace_back(ExecutionContext::interpret);
  ec.input_files.emplace_back(filepath);
  auto exec = loxo_main(3, nullptr, ec);
  return exec ? std::make_pair(exec,
                               ec.output_stream.str() + ec.error_stream.str())
              : std::make_pair(exec, ec.output_stream.str());
}
} // namespace
TEST(DISABLED_function, native1) {
  const auto path = LOXO_ROOT_DIR R"(\examples\fn\native1.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "Plenty of time remaining: \n2\nGetting closer: \n1\nCountdown "
            "complete!\n");
  EXPECT_EQ(callback, 0);
}

TEST(function, custom1) {
  const auto path = LOXO_ROOT_DIR R"(\examples\fn\custom1.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "2601\n");
  EXPECT_EQ(callback, 0);
}

TEST(function, return1) {
  const auto path = LOXO_ROOT_DIR R"(\examples\fn\return1.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(
      str,
      "can see me\n3\ndivide by 0 is not allowed. will show 'nan'.\nnan\n");
  EXPECT_EQ(callback, 0);
}

TEST(DISABLED_function, resurse1) {
  const auto path = LOXO_ROOT_DIR R"(\examples\fn\recurse1.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "55\ntrue\n");
  EXPECT_EQ(callback, 0);
}

TEST(function, withargs) {
  const auto path = LOXO_ROOT_DIR R"(\examples\fn\withargs.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "foo\n");
  EXPECT_EQ(callback, 0);
}

TEST(function, nested1) {
  const auto path = LOXO_ROOT_DIR R"(\examples\fn\nested1.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "Numbers >= 55:\n55\n56\n57\n58\n59\nNumbers >= "
            "10:\n10\n11\n12\n13\n14\n");
  EXPECT_EQ(callback, 0);
}

TEST(function, nested2) {
  const auto path = LOXO_ROOT_DIR R"(\examples\fn\nested2.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "6\n6\n");
  EXPECT_EQ(callback, 0);
}

TEST(function, scope1) {
  const auto path = LOXO_ROOT_DIR R"(\examples\fn\scope1.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "3\n2\n1\nBlast off!\n");
  EXPECT_EQ(callback, 0);
}

TEST(function, scope2) {
  const auto path = LOXO_ROOT_DIR R"(\examples\fn\scope2.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "109\n109\n99\n109\n");
  EXPECT_EQ(callback, 0);
}

TEST(function, scope3) {
  const auto path = LOXO_ROOT_DIR R"(\examples\fn\scope3.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "Local x: \n11\nLocal y: \n19\nLocal x: \n12\nLocal y: \n18\nLocal "
            "x: \n13\nLocal y: \n17\nx is less than y:\n1\n2\n");
  EXPECT_EQ(callback, 0);
}

TEST(function, error1) {
  const auto path = LOXO_ROOT_DIR R"(\examples\fn\error1.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "Can only call functions and classes.\n[line 1]\n");
  EXPECT_EQ(callback, 70);
}

TEST(function, closure1) {
  const auto path = LOXO_ROOT_DIR R"(\examples\fn\closure1.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "debug: Starting\ndebug: Processing\ndebug: Finishing\ndebug: "
            "Extra line\nerror: Failed!\nerror: Retrying...\n");
  EXPECT_EQ(callback, 0);
}

TEST(function, closure2) {
  const auto path = LOXO_ROOT_DIR R"(\examples\fn\closure2.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "First:\n1\n2\n2\nFirst:\n2\n8\n8\nFirst:\n3\n11\n11\nFirst:"
            "\n4\n15\n15\nreset\nSecond:\n1\n6\n6\nSecond:\n2\n10\n10\n");
  EXPECT_EQ(callback, 0);
}
