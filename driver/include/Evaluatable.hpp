#ifndef AC_LOXO_EVALUATABLE_HPP
#define AC_LOXO_EVALUATABLE_HPP

#include <algorithm>
#include <cmath>
#include <compare>
#include <cstdint>
#include <limits>
#include <memory>
#include <random>
#include <utility>
#include <vector>
#include <functional>

#include <net/ancillarycat/utils/Variant.hpp>

#include "details/loxo_fwd.hpp"
#include "details/IVisitor.hpp"
#include "Token.hpp"

namespace net::ancillarycat::loxo::evaluation {

/// @brief A class that represents an evaluatable object
/// @interface Evaluatable
/// @implements utils::Printable
class Evaluatable : public utils::Printable {
public:
  using eval_result_t = utils::IVisitor::eval_result_t;

public:
  constexpr Evaluatable() = default;
  explicit constexpr Evaluatable(const uint_least32_t line) : line(line) {}
  virtual ~Evaluatable() override = default;
  auto operator=(const Evaluatable &) -> Evaluatable & = default;
  uint_least32_t get_line() const { return line; }

private:
  uint_least32_t line = std::numeric_limits<uint_least32_t>::quiet_NaN();
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

class LOXO_API Boolean : public Value, public utils::Viewable {
public:
  constexpr Boolean() = default;
  constexpr Boolean(bool value,
                    const uint_least32_t line =
                        std::numeric_limits<uint_least32_t>::quiet_NaN())
      : Value(line), value(value) {}
  constexpr Boolean(long double,
                    const uint_least32_t line =
                        std::numeric_limits<uint_least32_t>::quiet_NaN())
      : Value(line), value(true) {}
  Boolean(const Boolean &);
  Boolean &operator=(const Boolean &);
  Boolean(Boolean &&) noexcept;
  Boolean &operator=(Boolean &&) noexcept;
  auto operator==(const Boolean &) const -> Boolean;
  static auto make_true(uint_least32_t) -> Boolean;
  static auto make_false(uint_least32_t) -> Boolean;
  auto is_true() const noexcept -> bool;
  virtual ~Boolean() = default;

private:
  auto to_string_impl(const utils::FormatPolicy &) const
      -> string_type override;
  auto to_string_view_impl(const utils::FormatPolicy &) const
      -> string_view_type override;

private:
  std::optional<bool> value = std::nullopt;
} static inline LOXO_CONSTEXPR_IF_NOT_MSVC True{true, 0}, False{false, 0};

class LOXO_API Nil : public Value, public utils::Viewable {
public:
  constexpr Nil() = default;
  explicit Nil(const uint_least32_t line) : Value(line) {}
  Nil(const Nil &) = default;
  Nil(Nil &&) noexcept {}
  Nil &operator=(const Nil &);
  Nil &operator=(Nil &&) noexcept;
  virtual ~Nil() = default;

private:
  auto to_string_impl(const utils::FormatPolicy &) const
      -> string_type override;
  auto to_string_view_impl(const utils::FormatPolicy &) const
      -> string_view_type override;
} static inline LOXO_CONSTEXPR_IF_NOT_MSVC NilValue{};

class String : public Evaluatable, public utils::Viewable {
public:
  constexpr String() = default;
  explicit String(
      const string_type &,
      uint_least32_t line = std::numeric_limits<uint_least32_t>::quiet_NaN());
  explicit String(
      string_view_type,
      uint_least32_t line = std::numeric_limits<uint_least32_t>::quiet_NaN());
  explicit String(
      string_type &&,
      uint_least32_t line =
          std::numeric_limits<uint_least32_t>::quiet_NaN()) noexcept;
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

private:
  auto to_string_impl(const utils::FormatPolicy &) const
      -> string_type override;
  auto to_string_view_impl(const utils::FormatPolicy &) const
      -> string_view_type override;

private:
  string_type value;
};

class Number : public Value {
public:
  constexpr Number() = default;
  Number(
      long double,
      uint_least32_t line = std::numeric_limits<uint_least32_t>::quiet_NaN());
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

private:
  auto to_string_impl(const utils::FormatPolicy &) const
      -> string_type override;
};
class Callable : public Evaluatable {
  struct Function {
    using token_t = Token;
    using stmt_ptr_t = std::shared_ptr<statement::Stmt>;
    string_type name;
    std::vector<string_type> parameters;
    std::vector<stmt_ptr_t> body;
  };

public:
  enum type_t : uint8_t {
    kOrdinary = 0,
    kClosure = 1,
  };

public:
  using args_t = std::vector<utils::IVisitor::variant_type>;
  using string_view_type = utils::Viewable::string_view_type;
  using native_function_t = std::function<utils::IVisitor::variant_type(
      const interpreter &, args_t &)>;
  using custom_function_t = Function;
  using function_t =
      utils::Variant<utils::Monostate, native_function_t, custom_function_t>;

public:
  Callable() = default;
  virtual ~Callable() = default;

private:
  Callable(unsigned, native_function_t &&);
  Callable(unsigned, custom_function_t &&, type_t);

public:
  static auto create_custom(unsigned, custom_function_t &&, type_t) -> Callable;
  static auto create_native(unsigned, native_function_t &&) -> Callable;

public:
  constexpr inline auto arity() const -> unsigned { return my_arity; }

public:
  auto call(const interpreter &, args_t &&) const -> eval_result_t;

private:
  // dont support static variables in this function
  unsigned my_arity = std::numeric_limits<unsigned>::quiet_NaN();
  function_t my_function{utils::Monostate{}};
  type_t my_type = kOrdinary;

private:
  static constexpr auto native_signature = "<native fn>"sv;

private:
  auto to_string_impl(const utils::FormatPolicy &) const
      -> string_type override;
};

} // namespace net::ancillarycat::loxo::evaluation
#endif // AC_LOXO_EVALUATABLE_HPP
