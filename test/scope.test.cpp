#include <gtest/gtest.h>
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

TEST(scope, func_global) {
  const auto path = LOXO_ROOT_DIR R"(\examples\scope\func.global.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "global\nglobal\n");
  EXPECT_EQ(callback, 0);
}

TEST(scope, func_local) {
  const auto path = LOXO_ROOT_DIR R"(\examples\scope\func.local.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "global\nglobal\n");
  EXPECT_EQ(callback, 0);
}

TEST(scope, func_nested) {
  const auto path = LOXO_ROOT_DIR R"(\examples\scope\func.nested.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "1\n2\n3\n");
  EXPECT_EQ(callback, 0);
}

TEST(scope, self_init_global) {
  const auto path = LOXO_ROOT_DIR R"(\examples\scope\self.init.global.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "value\n");
  EXPECT_EQ(callback, 0);
}

TEST(scope, self_init_local) {
  const auto path = LOXO_ROOT_DIR R"(\examples\scope\self.init.local.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "[line 3] Error at 'a': "
            "Can't read local variable in its own initializer.\n");
  EXPECT_EQ(callback, 65);
}

TEST(scope, self_init_wrapped) {
  const auto path = LOXO_ROOT_DIR R"(\examples\scope\self.init.wrapped.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "[line 8] Error at 'b': "
            "Can't read local variable in its own initializer.\n");
  EXPECT_EQ(callback, 65);
}
TEST(scope, self_init_nested) {
  const auto path = LOXO_ROOT_DIR R"(\examples\scope\self.init.nested.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "[line 5] Error at 'a': "
            "Can't read local variable in its own initializer.\n");
  EXPECT_EQ(callback, 65);
}
