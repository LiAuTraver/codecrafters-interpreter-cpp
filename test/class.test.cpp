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

TEST(class, decl_global1) {
  const auto path = LOXO_ROOT_DIR R"(\examples\class\decl.global1.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "Spaceship\n");
  EXPECT_EQ(callback, 0);
}
TEST(class, decl_global2) {
  const auto path = LOXO_ROOT_DIR R"(\examples\class\decl.global2.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "Robot\n"
            "Wizard\n"
            "Both classes successfully printed\n");
  EXPECT_EQ(callback, 0);
}

TEST(class, decl_local){
  const auto path = LOXO_ROOT_DIR R"(\examples\class\decl.local.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "Inside block: Dinosaur exists\n"
            "Dinosaur\n"
            "Accessing out-of-scope class:\n"
            "Undefined variable 'Dinosaur'.\n"
            "[line 8]\n");
  EXPECT_EQ(callback, 70);
}
TEST(class, decl_func) {
  const auto path = LOXO_ROOT_DIR R"(\examples\class\decl.func.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "Class declared inside function\n"
            "Superhero\n"
            "Function called successfully\n");
  EXPECT_EQ(callback, 0);
}
