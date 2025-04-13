#include <gtest/gtest.h>
#include "test_env.hpp"

namespace {
auto get_result(const auto &filepath) {
  ExecutionContext ec;
  ec.commands.emplace_back(ExecutionContext::interpret);
  ec.input_files.emplace_back(filepath);
  auto exec = accat::lox::main(3, nullptr, ec);
  return exec ? std::make_pair(exec,
                               ec.output_stream.str() + ec.error_stream.str())
              : std::make_pair(exec, ec.output_stream.str());
}
} // namespace

TEST(scope, func_global) {
  const auto path = LOX_ROOT_DIR R"(\examples\scope\func.global.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "global\nglobal\n");
  EXPECT_EQ(callback, 0);
}

TEST(scope, func_local) {
  const auto path = LOX_ROOT_DIR R"(\examples\scope\func.local.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "global\nglobal\n");
  EXPECT_EQ(callback, 0);
}

TEST(scope, func_nested) {
  const auto path = LOX_ROOT_DIR R"(\examples\scope\func.nested.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "1\n2\n3\n");
  EXPECT_EQ(callback, 0);
}

TEST(scope, self_init_global) {
  const auto path = LOX_ROOT_DIR R"(\examples\scope\self.init.global.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "value\n");
  EXPECT_EQ(callback, 0);
}

TEST(scope, self_init_local) {
  const auto path = LOX_ROOT_DIR R"(\examples\scope\self.init.local.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "[line 3] Error at 'a': "
            "Can't read local variable in its own initializer.\n");
  EXPECT_EQ(callback, 65);
}

TEST(scope, self_init_wrapped) {
  const auto path = LOX_ROOT_DIR R"(\examples\scope\self.init.wrapped.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "[line 8] Error at 'b': "
            "Can't read local variable in its own initializer.\n");
  EXPECT_EQ(callback, 65);
}
TEST(scope, self_init_nested) {
  const auto path = LOX_ROOT_DIR R"(\examples\scope\self.init.nested.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "[line 5] Error at 'a': "
            "Can't read local variable in its own initializer.\n");
  EXPECT_EQ(callback, 65);
}

TEST(scope, redefine_func) {
  const auto path = LOX_ROOT_DIR R"(\examples\scope\redefine.func.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "[line 2] Error at 'a': "
            "Already a variable with this name in this scope.\n");
  EXPECT_EQ(callback, 65);
}
TEST(scope, redefine_local) {
  const auto path = LOX_ROOT_DIR R"(\examples\scope\redefine.local.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "[line 3] Error at 'a': "
            "Already a variable with this name in this scope.\n");
  EXPECT_EQ(callback, 65);
}
TEST(scope, redefine_mixed) {
  const auto path = LOX_ROOT_DIR R"(\examples\scope\redefine.mixed.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "[line 12] Error at 'a': "
            "Already a variable with this name in this scope.\n");
  EXPECT_EQ(callback, 65);
}
TEST(scope, redefine_param) {
  const auto path = LOX_ROOT_DIR R"(\examples\scope\redefine.param.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "[line 1] Error at 'arg': "
            "Already a variable with this name in this scope.\n");
  EXPECT_EQ(callback, 65);
}

TEST(scope, return_block) {
  const auto path = LOX_ROOT_DIR R"(\examples\scope\return.block.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "[line 2] Error at 'return': Can't return from top-level code.\n");
  EXPECT_EQ(callback, 65);
}
TEST(scope, return_ctrlflow) {
  const auto path = LOX_ROOT_DIR R"(\examples\scope\return.ctrlflow.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "[line 12] Error at 'return': Can't return from top-level code.\n");
  EXPECT_EQ(callback, 65);
}

TEST(scope, return_global) {
  const auto path = LOX_ROOT_DIR R"(\examples\scope\return.global.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "[line 5] Error at 'return': Can't return from top-level code.\n");
  EXPECT_EQ(callback, 65);
}

TEST(scope, return_mixed) {
  const auto path = LOX_ROOT_DIR R"(\examples\scope\return.mixed.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "[line 14] Error at 'return': Can't return from top-level code.\n");
  EXPECT_EQ(callback, 65);
}
