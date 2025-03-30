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

TEST(scope, local_func) {
  const auto path = LOXO_ROOT_DIR R"(\examples\scope\local_func.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "global\nglobal\n");
  EXPECT_EQ(callback, 0);
}

TEST(scope, global_func) {
  const auto path = LOXO_ROOT_DIR R"(\examples\scope\global_func.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "global\nglobal");
  EXPECT_EQ(callback, 0);
}

TEST(scope, nested_func) {
  const auto path = LOXO_ROOT_DIR R"(\examples\scope\nested_func.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "1\n2\n3\n");
  EXPECT_EQ(callback, 0);
}