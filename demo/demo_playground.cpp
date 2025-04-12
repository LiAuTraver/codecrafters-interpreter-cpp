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
#include "../driver/include/expression.hpp"
using namespace accat::auxilia;

using namespace accat::loxo;

struct Base {
  virtual ~Base() = default;
  bool operator==(const Base &that) const {
    return typeid(that) == typeid(*this) && doEqual(*this, that);
  }
  // exposition only, should be private
  virtual bool doEqual(const Base &lhs, const Base &rhs) const = 0;
};
struct Derived1 : Base {
  bool doEqual(const Base &lhs, const Base &rhs) const override {
    return static_cast<const Derived1 &>(lhs).i ==
           static_cast<const Derived1 &>(rhs).i;
  }
  int i = 0;
};
struct Derived2 : Base {
  bool doEqual(const Base &lhs, const Base &rhs) const override {
    return static_cast<const Derived2 &>(lhs).d ==
           static_cast<const Derived2 &>(rhs).d;
  }
  double d = 0.0;
};


template <typename SmartPtr>
concept SmartPtrLike = requires(SmartPtr ptr) {
  { ptr.get() } -> std::same_as<typename SmartPtr::element_type *>;
  { ptr.operator->() } -> std::same_as<typename SmartPtr::element_type *>;
  { ptr.operator*() } -> std::same_as<typename SmartPtr::element_type &>;
};
template <SmartPtrLike SmartPtr> struct Hasher {
  std::size_t operator()(const SmartPtr &ptr) const {
    return std::hash<std::uintptr_t>()(
        reinterpret_cast<std::uintptr_t>(ptr.get()));
  }
};

template <SmartPtrLike SmartPtr> struct Equal {
  bool operator()(const SmartPtr &lhs, const SmartPtr &rhs) const {
    return *lhs == *rhs;
  }
};

using Map = std::unordered_map<std::shared_ptr<Base>, size_t>;

using NewMap = std::unordered_map<std::shared_ptr<Base>,
                                  size_t,
                                  Hasher<std::shared_ptr<Base>>,
                                  Equal<std::shared_ptr<Base>>>;

int main() {
  {
    Map my_map;
    auto derived1 = std::make_shared<Derived1>();
    auto derived2 = std::make_shared<Derived2>();
    my_map[derived1] = 1;
    my_map[derived2] = 2;
    std::cout << "derived1: " << my_map[derived1] << "\n";
    std::cout << "derived2: " << my_map[derived2] << "\n";

    auto anotherDerived1 = std::make_shared<Derived1>();

    auto it = my_map.find(anotherDerived1);
    if (it != my_map.end()) {
      std::cout << "Found anotherDerived1 in the map with value: " << it->second
                << "\n";
    } else {
      std::cout << "anotherDerived1 not found in the map\n";
    }
  }
  {
    NewMap my_map;
    auto derived1 = std::make_shared<Derived1>();
    auto derived2 = std::make_shared<Derived2>();
    my_map[derived1] = 1;
    my_map[derived2] = 2;
    std::cout << "derived1: " << my_map[derived1] << "\n";
    std::cout << "derived2: " << my_map[derived2] << "\n";

    auto anotherDerived1 = std::make_shared<Derived1>();
    auto it = my_map.find(anotherDerived1);
    if (it != my_map.end()) { // true
      std::cout << "Found anotherDerived1 in the map with value: " << it->second
                << "\n";
    } else {
      std::cout << "anotherDerived1 not found in the map\n";
    }

    auto anotherDerived2 = std::make_shared<Derived2>();
    anotherDerived2->d = 42.0;
    auto it2 = my_map.find(anotherDerived2);
    if (it2 != my_map.end()) { // false
      std::cout << "Found anotherDerived2 in the map with value: " << it2->second
                << "\n";
    } else {
      std::cout << "anotherDerived2 not found in the map\n";
    }
  }
}
