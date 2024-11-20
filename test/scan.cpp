#include <gtest/gtest.h>
#include <filesystem>
#include <string_view>
#include "../src/loxograph_driver.cpp"

using namespace net::ancillarycat::loxograph;
using namespace std::string_view_literals;
using std::filesystem::path;
const auto command = "tokenize"sv;

auto get_result(auto &filepath, auto &command) {
  std::ostringstream oss;
  loxo_main(filepath, command, oss);
  return oss.str();
}

TEST(scan, invalid_path) {
  const auto filepath = path(R"(ABCDEF)");
  auto result = get_result(filepath, command);
  EXPECT_EQ(result, "");
}

TEST(scan, empty_file) {
  const auto filepath = path(R"(Z:/loxograph/templates/empty.lox)");
  auto result = get_result(filepath, command);
  EXPECT_EQ(result, "EOF  null\n");
}

TEST(scan, simple1) {
  const auto filepath = path(R"(Z:/loxograph/templates/simple1.lox)");
  auto result = get_result(filepath, command);
  EXPECT_EQ(result, "VAR var null\n"
                    "IDENTIFIER language null\n"
                    "EQUAL = null\n"
                    "STRING \"lox\" lox\n"
                    "SEMICOLON ; null\n"
                    "EOF  null\n");
}

TEST(scan, simple2) {
  const auto filepath = path(R"(Z:/loxograph/templates/simple2.lox)");
  auto result = get_result(filepath, command);
  EXPECT_EQ(result, "LEFT_PAREN ( null\n"
                    "LEFT_PAREN ( null\n"
                    "RIGHT_PAREN ) null\n"
                    "EOF  null\n");
}

TEST(scan, simple3) {
  const auto filepath = path(R"(Z:/loxograph/templates/simple3.lox)");
  auto result = get_result(filepath, command);
  EXPECT_EQ(result, "LEFT_PAREN ( null\n"
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