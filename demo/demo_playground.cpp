#include <expected>
#include <iostream>
#include <ostream>
#include <string>
#include <optional>
#include <variant>
#include <fmt/format.h>
#include <print>

#include <accat/auxilia/auxilia.hpp>

using namespace accat::auxilia;
int main() {
  Variant<Monostate, int, std::string> v = 1;
  if (auto ptr = v.get_if<int>()) {
    std::println("int: {}", *ptr);
  }
  v = "Hello world!"s;
  if (v.is_type<std::string>()) {
    std::println("string: {}", v.get<std::string>());
  }
  v = Monostate{}; // reset to monostate, or v.reset()
  v.visit(match([](const int &i) { std::println("int: {}", i); },
                [](const std::string &s) { std::println("string: {}", s); },
                [](const auto &) { std::println("don't care"); } // catch-all
                ));
  fmt::println("v: {}", v); // prints "v: Monostate"
}
