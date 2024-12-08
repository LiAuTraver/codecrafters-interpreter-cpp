#include <gtest/gtest.h>
#include "test_env.hpp"

auto get_result(const auto &filepath) {
  std::ostringstream oss;
  ExecutionContext ec;
  ec.commands.push_back(ExecutionContext::lex);
  ec.input_files.push_back(filepath);
  ec.output_stream.set_rdbuf(oss.rdbuf());
  auto _ = loxo_main(3, nullptr, ec);
  return oss.str();
}

TEST(scan, invalid_path) {
  const auto filepath = path(R"(ABCDEF)");
  auto result = get_result(filepath);
  EXPECT_EQ(result, "");
}

TEST(scan, empty_file) {
  const auto filepath = path(R"(Z:/loxo/examples/scanning/empty.lox)");
  auto result = get_result(filepath);
  EXPECT_EQ(result, "EOF  null\n");
}

TEST(scan, simple1) {
  const auto filepath = path(R"(Z:/loxo/examples/scanning/simple1.lox)");
  auto result = get_result(filepath);
  EXPECT_EQ(result,
            "VAR var null\n"
            "IDENTIFIER language null\n"
            "EQUAL = null\n"
            "STRING \"lox\" lox\n"
            "SEMICOLON ; null\n"
            "EOF  null\n");
}

TEST(scan, simple2) {
  const auto filepath = path(R"(Z:/loxo/examples/scanning/simple2.lox)");
  auto result = get_result(filepath);
  EXPECT_EQ(result,
            "LEFT_PAREN ( null\n"
            "LEFT_PAREN ( null\n"
            "RIGHT_PAREN ) null\n"
            "EOF  null\n");
}

TEST(scan, simple3) {
  const auto filepath = path(R"(Z:/loxo/examples/scanning/simple3.lox)");
  auto result = get_result(filepath);
  EXPECT_EQ(result,
            "LEFT_PAREN ( null\n"
            "LEFT_BRACE { null\n"
            "STAR * null\n"
            "DOT . null\n"
            "COMMA , null\n"
            "PLUS + null\n"
            "STAR * null\n"
            "RIGHT_BRACE } null\n"
            "RIGHT_PAREN ) null\n"
            "EOF  null\n");
}
