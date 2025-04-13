#pragma once

#include <algorithm>
#include <cmath>
#include <compare>
#include <cstdint>
#include <limits>
#include <memory>
#include <random>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>
#include <functional>

#include <accat/auxilia/auxilia.hpp>

#include "details/lox_fwd.hpp"
#include "details/IVisitor.hpp"

namespace accat::lox::evaluation {

/// @brief A class that represents an evaluatable object
/// @interface Evaluatable
/// @implements auxilia::Printable
class Evaluatable : public auxilia::Printable {
protected:
  static constexpr auto nan =
      &std::numeric_limits<uint_least32_t>::signaling_NaN;

public:
  using eval_result_t = IVisitor::eval_result_t;

public:
  constexpr Evaluatable() = default;
  explicit constexpr Evaluatable(const uint_least32_t line) : line(line) {}
  virtual ~Evaluatable() = default;
  auto operator=(const Evaluatable &) -> Evaluatable & = default;
  uint_least32_t get_line() const { return line; }

public:
  virtual auto to_string(const auxilia::FormatPolicy &) const
      -> string_type = 0;

private:
  uint_least32_t line = nan();
};
/// @brief A class that represents a callable object
class Callable {
protected:
  virtual ~Callable() = default;

public:
  using args_t = std::vector<IVisitor::variant_type>;
  using env_t = Environment;
  using env_ptr_t = std::shared_ptr<env_t>;

public:
  virtual auto arity() const -> unsigned = 0;
  virtual auto call(interpreter &, args_t &&) -> Evaluatable::eval_result_t = 0;
};

/// @brief A class that represents a value
/// @interface Value
/// @implements Evaluatable
class Value : public Evaluatable {
public:
  constexpr Value() = default;
  constexpr explicit Value(const uint_least32_t line) : Evaluatable(line) {}
  virtual ~Value() override = default;
  explicit operator Boolean() const noexcept;
  Boolean operator!() const noexcept;
};

class AC_LOX_API Boolean : public Value, public auxilia::Viewable {
public:
  constexpr Boolean() = default;
  constexpr Boolean(bool value, const uint_least32_t line = nan())
      : Value(line), value(value) {}
  constexpr Boolean(long double, const uint_least32_t line = nan())
      : Value(line), value(true) {}
  Boolean(const Boolean &);
  Boolean &operator=(const Boolean &);
  Boolean(Boolean &&) noexcept;
  Boolean &operator=(Boolean &&) noexcept;
  static auto make_true(uint_least32_t) -> Boolean;
  static auto make_false(uint_least32_t) -> Boolean;
  auto is_true() const noexcept -> bool;
  virtual ~Boolean() = default;

public:
  auto to_string(const auxilia::FormatPolicy &) const -> string_type override;
  auto to_string_view(const auxilia::FormatPolicy &) const -> string_view_type;

public:
  Boolean operator==(const Boolean &rhs) const {
    return (this->value == rhs.value);
  }
  Boolean operator!=(const Boolean &rhs) const {
    return (this->value != rhs.value);
  }

private:
  bool value = false;
} static inline AC_CONSTEXPR20 True{true, 0}, False{false, 0};

class AC_LOX_API Nil : public Value, public auxilia::Viewable {
public:
  constexpr Nil() = default;
  explicit Nil(const uint_least32_t line) : Value(line) {}
  Nil(const Nil &) = default;
  Nil(Nil &&) noexcept {}
  Nil &operator=(const Nil &);
  Nil &operator=(Nil &&) noexcept;
  virtual ~Nil() = default;

public:
  auto to_string(const auxilia::FormatPolicy &) const -> string_type;
  auto to_string_view(const auxilia::FormatPolicy &) const -> string_view_type;
} static inline AC_CONSTEXPR20 NilValue{};

class String : public Evaluatable, public auxilia::Viewable {
public:
  constexpr String() = default;
  explicit String(const string_type &, uint_least32_t line = nan());
  explicit String(string_view_type, uint_least32_t line = nan());
  explicit String(string_type &&, uint_least32_t line = nan()) noexcept;
  String(const String &);
  String(String &&) noexcept;
  String &operator=(const String &);
  String &operator=(String &&) noexcept;
  virtual ~String() override = default;

public:
  String operator+(const String &) const;
  Boolean operator==(const String &) const;
  Boolean operator!=(const String &) const;
  explicit operator Boolean() const;

public:
  auto to_string(const auxilia::FormatPolicy &) const -> string_type override {
    return value;
  }
  auto to_string_view(const auxilia::FormatPolicy &) const -> string_view_type {
    return value;
  }

private:
  string_type value;
};

class Number : public Value {
public:
  constexpr Number() = default;
  Number(long double, uint_least32_t line = nan());
  Number(const Number &);
  Number(Number &&) noexcept;
  Number &operator=(const Number &);
  Number &operator=(Number &&) noexcept;
  virtual ~Number() override = default;

public:
  Boolean operator==(const Number &) const;
  Boolean operator!=(const Number &) const;
  Boolean operator<(const Number &) const;
  Boolean operator<=(const Number &) const;
  Boolean operator>(const Number &) const;
  Boolean operator>=(const Number &) const;
  Number operator-(const Number &) const;
  Number operator+(const Number &) const;
  Number operator*(const Number &) const;
  Number operator/(const Number &) const;
  Number &operator+=(const Number &);
  Number &operator-=(const Number &);
  Number &operator*=(const Number &);
  Number &operator/=(const Number &);

private:
  long double value = std::numeric_limits<long double>::quiet_NaN();

public:
  auto to_string(const auxilia::FormatPolicy &) const -> string_type override;
};
class Function : public Evaluatable, public Callable {
  struct RealFunction {
    using stmt_ptr_t = std::shared_ptr<statement::Stmt>;
    string_type name;
    std::vector<string_type> parameters;
    std::vector<stmt_ptr_t> body;
  };

public:
  using string_view_type = auxilia::string_view;
  using native_function_t =
      std::function<IVisitor::variant_type(interpreter &, args_t &)>;
  using custom_function_t = RealFunction;
  using function_t = auxilia::
      Variant<auxilia::Monostate, native_function_t, custom_function_t>;

public:
  Function() = default;
  Function(const Function &) = default;
  Function &operator=(const Function &) = default;
  Function(Function &&) noexcept;
  Function &operator=(Function &&) noexcept;
  virtual ~Function() = default;

private:
  Function(unsigned, native_function_t &&, const env_ptr_t &, bool = false);
  Function(unsigned, custom_function_t &&, const env_ptr_t &, bool = false);
  Function(unsigned, const function_t &, const env_ptr_t &, bool = false);

public:
  static auto
  create_custom(unsigned, custom_function_t &&, const env_ptr_t &, bool = false)
      -> Function;
  static auto
  create_native(unsigned, native_function_t &&, const env_ptr_t &, bool = false)
      -> Function;
  auto bind(const Instance &) const -> Function;

public:
  virtual inline auto arity() const -> unsigned override { return my_arity; }
  virtual auto call(interpreter &, args_t &&) -> eval_result_t override;

private:
  // dont support static variables in this function
  unsigned my_arity = std::numeric_limits<unsigned>::quiet_NaN();
  function_t my_function;
  env_ptr_t my_env;
  bool is_initializer = false;

private:
  static constexpr auto native_signature = "<native fn>"sv;

public:
  auto to_string(const auxilia::FormatPolicy &) const -> string_type override;

private:
  friend inline auto operator==(const Function &lhs, const Function &rhs)
      -> bool {
    return lhs.my_arity == rhs.my_arity &&
           lhs.my_function.index() == rhs.my_function.index() &&
           std::addressof(*lhs.my_env) == std::addressof(*rhs.my_env);
  }
  friend inline auto operator!=(const Function &lhs, const Function &rhs)
      -> bool {
    return !(lhs == rhs);
  }
};

class Class : public Evaluatable, public Callable {
public:
  using methods_t = std::unordered_map<string_type, Function>;
  string_type name;
  methods_t methods;

public:
  Class(std::string_view, uint_least32_t, methods_t && = {});

public:
  auto arity() const -> unsigned override;
  auto call(interpreter &, args_t &&) -> eval_result_t override;

public:
  auto get_method(std::string_view) const -> auxilia::StatusOr<Function>;

public:
  auto to_string(const auxilia::FormatPolicy &) const -> string_type override;

private:
  auto get_initializer() [[clang::lifetimebound]] -> Function *;

private:
  friend inline auto operator==(const Class &lhs, const Class &rhs) -> bool {
    return lhs.name == rhs.name && lhs.methods == rhs.methods;
  }
  friend inline auto operator!=(const Class &lhs, const Class &rhs) -> bool {
    return !(lhs == rhs);
  }
};
class Instance : public Evaluatable {
  using field_t = std::pair<string_type, eval_result_t>;
  using fields_t = std::unordered_map<string_type, eval_result_t>;
  using fields_ptr_t = std::shared_ptr<fields_t>;
  using env_t = Callable::env_t;
  using env_ptr_t = Callable::env_ptr_t;

private:
  /// @note the current design flaw was rather big, storing value in variant as
  /// result was great until class and instance comes in, which is stateful and
  /// if copied, we're changing the state of the copy, not the original.
  ///
  /// Currently i have neither no desire nor the time to change it, so I have to
  /// use shared_ptr to store class_env and class fields.
  ///
  /// @todo weak_ptr seems not to be a proper choice!
  /// If shared_ptr is used, then the env and values stored may have cyclic
  /// strong reference: it'll fine for Function holds a strong ref to env, and
  /// env holds a weak_ref to func, but its not the case for pure values like
  /// String, Number etc; while the values are stored in env probably in a
  /// type-erasure way.
  fields_ptr_t fields;
  /// @note this is a bit of a hack, but it works for now.
  /// We ought to store a (strong) reference to the Class object, but the class
  /// object was inside variant and the unordered_map might move it when
  /// rehashing; if the env was destroyed, the class object would be destroyed
  /// too, but in language like js and C++, the class still works even if the
  /// class was not visible outside a scope (in a word, classes defined in scope
  /// only affect its usage for the user, restricting it in a specific scope,
  /// much like a define-ed and undef-ed macro in C), the instance might
  /// outlive. So we have to make it a strong reference.
  env_ptr_t class_env;
  /// @note class name won't change(stateless), so we can store it as a value.
  string_type class_name;

public:
  explicit Instance(const env_ptr_t &, std::string_view);

public:
  auto get_field(std::string_view) const -> eval_result_t;
  auto set_field(std::string_view, eval_result_t &&, bool = false)
      -> auxilia::Status;
  auto to_string(const auxilia::FormatPolicy &) const -> string_type override;

private:
  auto get_class() const -> Class &;

private:
  friend auto operator==(const Instance &lhs, const Instance &rhs) {
    return lhs.get_class() == rhs.get_class()
        // && lhs.fields == rhs.fields  // compile error
        ;
  }
  friend auto operator!=(const Instance &lhs, const Instance &rhs) {
    return !(lhs == rhs);
  }
};
} // namespace accat::lox::evaluation
