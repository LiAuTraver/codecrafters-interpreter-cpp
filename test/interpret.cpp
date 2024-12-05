#include <gtest/gtest.h>
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

TEST(interpret, print) {
  const auto path = R"(Z:\loxo\examples\interp.stmt1.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "Hello, World!\n42\ntrue\n36\n");
  EXPECT_EQ(callback, 0);
}

TEST(interpret, error) {
  const auto path = R"(Z:\loxo\examples\interp.expr2.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "the expression below is invalid\nOperands must be two numbers or "
            "two strings.\n[line 2]\n");
  EXPECT_EQ(callback, 70);
}

TEST(interpret, variable_print) {
  const auto path = R"(Z:\loxo\examples\interp.var1.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "10\n");
  EXPECT_EQ(callback, 0);
}

TEST(interpret, variable_print2) {
  const auto path = R"(Z:\loxo\examples\interp.var2.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "198\n297\n");
  EXPECT_EQ(callback, 0);
}

TEST(interpret, variable_print3) {
  const auto path = R"(Z:\loxo\examples\interp.var3.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "310\n");
  EXPECT_EQ(callback, 0);
}

TEST(interpret, variable_print4) {
  const auto path = R"(Z:\loxo\examples\interp.var4.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "152\n");
  EXPECT_EQ(callback, 0);
}

TEST(interpret, error1) {
  const auto path = R"(Z:\loxo\examples\interp.err1.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "Undefined variable 'a'.\n[line 1]\n");
  EXPECT_EQ(callback, 70);
}
TEST(interpret, error2) {
  const auto path = R"(Z:\loxo\examples\interp.err2.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "22\nUndefined variable 'x'.\n[line 2]\n");
  EXPECT_EQ(callback, 70);
}
TEST(interpret, error3) {
  const auto path = R"(Z:\loxo\examples\interp.err3.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "Undefined variable 'hello'.\n[line 2]\n");
  EXPECT_EQ(callback, 70);
}
TEST(interpret, error4) {
  const auto path = R"(Z:\loxo\examples\interp.err4.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "Undefined variable 'bar'.\n[line 2]\n");
  EXPECT_EQ(callback, 70);
}
TEST(interpret, error5) {
  const auto path = R"(Z:\loxo\examples\interp.err5.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "[line 4] Error at '': Expect expression.\n");
  EXPECT_EQ(callback, 65);
}

TEST(interpret, nil) {
  const auto path = R"(Z:\loxo\examples\interp.nil.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "593\n1113\nnil\n");
  EXPECT_EQ(callback, 0);
}

TEST(interpret, redef) {
  const auto path = R"(Z:\loxo\examples\interp.redef.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "before\n");
  EXPECT_EQ(callback, 0);
}

TEST(interpret, reassign1) {
  const auto path = R"(Z:\loxo\examples\interp.reassign1.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "1\n");
  EXPECT_EQ(callback, 0);
}

TEST(interpret, reassign2) {
  const auto path = R"(Z:\loxo\examples\interp.reassign2.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "1\n2\n2\n");
  EXPECT_EQ(callback, 0);
}

TEST(interpret, reassign3) {
  const auto path = R"(Z:\loxo\examples\interp.reassign3.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "186\n");
  EXPECT_EQ(callback, 0);
}

TEST(interpret, reassign4) {
  const auto path = R"(Z:\loxo\examples\interp.reassign4.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "3144\n3144\n");
  EXPECT_EQ(callback, 0);
}

TEST(interpret, reassign5) {
  const auto path = R"(Z:\loxo\examples\interp.reassign5.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "130\n130\n130\n");
  EXPECT_EQ(callback, 0);
}


TEST(interpret, scope1) {
  const auto path = R"(Z:\loxo\examples\interp.scope1.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "before\nafter\n");
  EXPECT_EQ(callback, 0);
}

TEST(interpret, scope2) {
  const auto path = R"(Z:\loxo\examples\interp.scope2.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "baz\n");
  EXPECT_EQ(callback, 0);
}

TEST(interpret, scope3) {
  const auto path = R"(Z:\loxo\examples\interp.scope3.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "88\n88\n");
  EXPECT_EQ(callback, 0);
}

TEST(interpret, scope4) {
  const auto path = R"(Z:\loxo\examples\interp.scope4.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "[line 6] Error at '': Expect '}'.\n");
  EXPECT_EQ(callback, 65);
}
