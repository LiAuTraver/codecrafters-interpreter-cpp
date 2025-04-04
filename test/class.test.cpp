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

TEST(class, decl_global) {
  const auto path = LOXO_ROOT_DIR R"(\examples\class\decl.global.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "Spaceship\n"
            "Robot\n"
            "Wizard\n"
            "Both classes successfully printed\n");
  EXPECT_EQ(callback, 0);
}

TEST(class, decl_local) {
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

TEST(class, instance_global) {
  const auto path = LOXO_ROOT_DIR R"(\examples\class\instance.global.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "Spaceship instance\n"
            "Created multiple robots:\n"
            "Robot instance\n"
            "Robot instance\n");
  EXPECT_EQ(callback, 0);
}

TEST(class, instance_local) {
  const auto path = LOXO_ROOT_DIR R"(\examples\class\instance.local.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "Characters created in fantasy world:\n"
            "Wizard instance\n"
            "Dragon instance\n"
            "The main character is:\n"
            "Wizard instance\n"
            "Hero created:\n"
            "Superhero instance\n"
            "Hero created:\n"
            "Superhero instance\n"
            "Hero created:\n"
            "Superhero instance\n"
            "All heroes created!\n");
  EXPECT_EQ(callback, 0);
}

TEST(class, property_basic) {
  const auto path = LOXO_ROOT_DIR R"(\examples\class\property.basic.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "Ship details:\n"
            "Millennium Falcon\n"
            "75.5\n"
            "Robot is not operational.\n");
  EXPECT_EQ(callback, 0);
}

TEST(class, property_multiple) {
  const auto path = LOXO_ROOT_DIR R"(\examples\class\property.multiple.lox)";
  auto [callback, str] = get_result(path);

  EXPECT_EQ(str,
            "Times Superman was called: \n"
            "66\n"
            "Times Batman was called: \n"
            "18\n");
  EXPECT_EQ(callback, 0);
}

TEST(class, property_manipulation) {
  const auto path =
      LOXO_ROOT_DIR R"(\examples\class\property.manipulation.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "Grey\n"
            "White\n"
            "100\n");
  EXPECT_EQ(callback, 0);
}
