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

TEST(class, decl_global) {
  const auto path = LOX_ROOT_DIR R"(\examples\class\decl.global.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "Spaceship\n"
            "Robot\n"
            "Wizard\n"
            "Both classes successfully printed\n");
  EXPECT_EQ(callback, 0);
}

TEST(class, decl_local) {
  const auto path = LOX_ROOT_DIR R"(\examples\class\decl.local.lox)";
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
  const auto path = LOX_ROOT_DIR R"(\examples\class\decl.func.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "Class declared inside function\n"
            "Superhero\n"
            "Function called successfully\n");
  EXPECT_EQ(callback, 0);
}

TEST(class, instance_global) {
  const auto path = LOX_ROOT_DIR R"(\examples\class\instance.global.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "Spaceship instance\n"
            "Created multiple robots:\n"
            "Robot instance\n"
            "Robot instance\n");
  EXPECT_EQ(callback, 0);
}

TEST(class, instance_local) {
  const auto path = LOX_ROOT_DIR R"(\examples\class\instance.local.lox)";
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
  const auto path = LOX_ROOT_DIR R"(\examples\class\property.basic.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "Ship details:\n"
            "Millennium Falcon\n"
            "75.5\n"
            "Robot is not operational.\n");
  EXPECT_EQ(callback, 0);
}

TEST(class, property_multiple) {
  const auto path = LOX_ROOT_DIR R"(\examples\class\property.multiple.lox)";
  auto [callback, str] = get_result(path);

  EXPECT_EQ(str,
            "Times Superman was called: \n"
            "66\n"
            "Times Batman was called: \n"
            "18\n");
  EXPECT_EQ(callback, 0);
}

TEST(class, property_manipulation) {
  const auto path = LOX_ROOT_DIR R"(\examples\class\property.manipulation.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "Grey\n"
            "White\n"
            "100\n");
  EXPECT_EQ(callback, 0);
}

TEST(class, methods_global) {
  const auto path = LOX_ROOT_DIR R"(\examples\class\methods.global.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "Beep boop!\n"
            "Beep boop!\n"
            "Casting a magical spell: Fireball\n");
  EXPECT_EQ(callback, 0);
}

TEST(class, methods_local) {
  const auto path = LOX_ROOT_DIR R"(\examples\class\methods.local.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "Foo\n");
  EXPECT_EQ(callback, 0);
}

TEST(class, methods_withArgs) {
  const auto path = LOX_ROOT_DIR R"(\examples\class\methods.withArgs.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "Using power: Flight\n");
  EXPECT_EQ(callback, 0);
}

TEST(class, this_basic) {
  const auto path = LOX_ROOT_DIR R"(\examples\class\this.basic.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "Spaceship instance\n");
  EXPECT_EQ(callback, 0);
}

TEST(class, this_property) {
  const auto path = LOX_ROOT_DIR R"(\examples\class\this.property.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "175\n"
            "Woof\n"
            "Cat\n");
  EXPECT_EQ(callback, 0);
}

TEST(class, this_nested) {
  const auto path = LOX_ROOT_DIR R"(\examples\class\this.nested.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "Wizard instance\n"
            "Casting spell as Merlin\n");
  EXPECT_EQ(callback, 0);
}

TEST(class, ctor_global) {
  const auto path = LOX_ROOT_DIR R"(\examples\class\ctor.global.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "bar\n"
            "91\n"
            "R2-D2\n"
            "startValue can't be negative\n"
            "0\n"
            "52\n"
            "Generic Generic\n"
            "Toyota Corolla with four wheels\n");
  EXPECT_EQ(callback, 0);
}

TEST(class, ctor_return1) {
  const auto path = LOX_ROOT_DIR R"(\examples\class\ctor.return1.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "world\n");
  EXPECT_EQ(callback, 0);
}

TEST(class, ctor_return2) {
  const auto path = LOX_ROOT_DIR R"(\examples\class\ctor.return2.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "[line 5] Error at 'return': Can't return a value from an "
            "initializer.\n");
  EXPECT_EQ(callback, 65);
}

TEST(class, ctor_return3) {
  const auto path = LOX_ROOT_DIR R"(\examples\class\ctor.return3.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "[line 3] Error at 'return': Can't return a value from an "
            "initializer.\n");
  EXPECT_EQ(callback, 65);
}

TEST(class, ctor_return4) {
  const auto path = LOX_ROOT_DIR R"(\examples\class\ctor.return4.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "[line 3] Error at 'return': Can't return a value from an "
            "initializer.\n");
  EXPECT_EQ(callback, 65);
}

TEST(class, inheritance_basic) {
  const auto path = LOX_ROOT_DIR R"(\examples\class\inheritance.basic.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "Doughnut instance\n"
            "BostonCream instance\n"
            "A instance\n"
            "B instance\n"
            "C instance\n"
            "Vehicle instance\n"
            "Car instance\n"
            "Sedan instance\n"
            "Truck instance\n");
  EXPECT_EQ(callback, 0);
}

TEST(class, inheritance_methods) {
  const auto path = LOX_ROOT_DIR R"(\examples\class\inheritance.methods.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "Fry until golden brown.\n"
            "Root class\n"
            "Root class\n"
            "Root class\n"
            "Method defined in Parent\n"
            "Method defined in Parent\n"
            "Method defined in Child\n");
  EXPECT_EQ(callback, 0);
}

TEST(class, inheritance_init) {
  const auto path = LOX_ROOT_DIR R"(\examples\class\inheritance.init.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "42\n");
  EXPECT_EQ(callback, 0);
}

TEST(class, symbols) {
  const auto path = LOX_ROOT_DIR R"(\examples\class\symbols.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "from foo\n"
            "from quz\n"
            "from hello\n");
  EXPECT_EQ(callback, 0);
}
TEST(class, inheritance_overrides) {
  const auto path = LOX_ROOT_DIR R"(\examples\class\inheritance.overrides.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "B method\n"
            "89\n"
            "32\n"
            "Animal speaks : Generic sound\n"
            "Dog speaks : Woof\n"
            "Puppy speaks : Woof\n");
  EXPECT_EQ(callback, 0);
}

TEST(class, inheritance_error1) {
  const auto path = LOX_ROOT_DIR R"(\examples\class\inheritance.error1.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "[line 2] Error at 'Foo': A class can't inherit from itself.\n");
  EXPECT_EQ(callback, 65);
}

TEST(class, inheritance_error2) {
  const auto path = LOX_ROOT_DIR R"(\examples\class\inheritance.error2.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "Superclass must be a class.\n"
            "[line 4]\n");
  EXPECT_EQ(callback, 70);
}

TEST(class, inheritance_error3) {
  const auto path = LOX_ROOT_DIR R"(\examples\class\inheritance.error3.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "Superclass must be a class.\n"
            "[line 4]\n");
  EXPECT_EQ(callback, 70);
}

TEST(class, inheritance_super) {
  const auto path = LOX_ROOT_DIR R"(\examples\class\inheritance.super.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "Fry until golden brown.\n"
            "Base.method()\n"
            "Base.method()\n");
  EXPECT_EQ(callback, 0);
}

TEST(class, super_error1) {
  const auto path = LOX_ROOT_DIR R"(\examples\class\super.error1.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str,
            "[line 4] Error at 'super': Can't use 'super' in a class with no "
            "superclass.\n");
  EXPECT_EQ(callback, 65);
}

TEST(class, super_error2) {
  const auto path = LOX_ROOT_DIR R"(\examples\class\super.error2.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "[line 7] Error at ';': Expect '.' after 'super'.\n");
  EXPECT_EQ(callback, 65);
}

TEST(class, super_error3) {
  const auto path = LOX_ROOT_DIR R"(\examples\class\super.error3.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(
      str,
      "[line 2] Error at 'super': Can't use 'super' outside of a class.\n");
  EXPECT_EQ(callback, 65);
}
