#include <expected>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <optional>
#include <variant>
#include <fmt/base.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <print>

#include <accat/auxilia/auxilia.hpp>

using namespace accat::auxilia;

class A {
public:
  A() = default;
  A(const A &) = default;
  A(A &&) = default;
  auto operator=(const A &) -> A & = default;
  auto operator=(A &&) -> A & = default;
  ~A() = default;
};
int A() { return 0; }


int main() {
  auto a = A();
  class A b{};
  return 0;
}
