#include <gtest/gtest.h>
#include <filesystem>
#include <string_view>
#include "loxo_main.hpp"

using namespace net::ancillarycat::loxograph;
using namespace std::string_view_literals;
using std::filesystem::path;
const auto command = "tokenize"sv;

TEST(scan, invalid_path) {
  const auto filepath = path(R"(ABCDEF)");
  auto result = loxo_main(filepath, command);
  EXPECT_EQ(result, "");
}

TEST(scan, empty_file) {
  const auto filepath = path(R"(Z:/loxograph/templates/empty.lox)");
  auto result = loxo_main(filepath, command);
  EXPECT_EQ(result, "EOF  null\n");
}

TEST(scan, simple1) {
  const auto filepath = path(R"(Z:/loxograph/templates/simple1.lox)");
  auto result = loxo_main(filepath, command);
  EXPECT_EQ(result, "VAR var null\n"
                    "IDENTIFIER language null\n"
                    "EQUAL = null\n"
                    "STRING \"lox\" lox\n"
                    "SEMICOLON ; null\n"
                    "EOF  null\n");
}

TEST(scan, simple2) {
  const auto filepath = path(R"(Z:/loxograph/templates/simple2.lox)");
  auto result = loxo_main(filepath, command);
  EXPECT_EQ(result, "LEFT_PAREN ( null\n"
                    "LEFT_PAREN ( null\n"
                    "RIGHT_PAREN ) null\n"
                    "EOF  null\n");
}

TEST(scan, simple3) {
  const auto filepath = path(R"(Z:/loxograph/templates/simple3.lox)");
  auto result = loxo_main(filepath, command);
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