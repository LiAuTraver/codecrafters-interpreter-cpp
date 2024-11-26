#include "config.hpp"
#include "loxo_fwd.hpp"
#include "fmt.hpp"

#include "Evaluatable.hpp"

namespace net::ancillarycat::loxograph::syntax {
auto Keyword::to_string_impl(const utils::FormatPolicy &) const -> string_type {
  return "not implemented"s;
}
auto Keyword::to_string_view_impl(const utils::FormatPolicy &) const
    -> string_view_type {
  return "not implemented"sv;
}
Value::operator Boolean() const noexcept {
  if (dynamic_cast<const Nil *>(this)) {
    return Boolean::make_false(get_line());
  }
  if (const auto ptr = dynamic_cast<const Boolean *>(this)) {
    return *ptr;
  }
  return Boolean::make_true(get_line());
}
Boolean Value::operator!() const noexcept {
  return Boolean{(!(this->operator Boolean().is_true()))};
}

Boolean::Boolean(const Boolean &value)
    : Value(value.get_line()), value(value.value) {}
Boolean &Boolean::operator=(const Boolean &value) {
  this->value = value.value;
  Evaluatable::operator=(value);
  return *this;
}
Boolean::Boolean(Boolean &&value) noexcept
    : Value(value.get_line()), value(value.value) {}
Boolean &Boolean::operator=(Boolean &&value) noexcept {
  this->value = value.value;
  Evaluatable::operator=(value);
  return *this;
}
auto Boolean::operator==(const Boolean &rhs) const -> Boolean {
  return Boolean{value == rhs.value};
}
auto Boolean::make_true(const uint_least32_t line) -> Boolean {
  return Boolean{true, line};
}
auto Boolean::make_false(const uint_least32_t line) -> Boolean {
  return Boolean{false, line};
}
bool Boolean::is_true() const noexcept {
  contract_assert(value.has_value() && (bool)"value is not set");
  return value.value();
}

auto Boolean::to_string_impl(const utils::FormatPolicy &format_policy) const
    -> string_type {
  contract_assert(value.has_value());
  return value.value() ? "true"s : "false"s;
}
auto Boolean::to_string_view_impl(
    const utils::FormatPolicy &format_policy) const -> string_view_type {
  contract_assert(value.has_value());
  return value.value() ? "true"sv : "false"sv;
}
Nil &Nil::operator=(const Nil &that) {
  if (this == &that)
    return *this;
  Evaluatable::operator=(that);
  return *this;
}
Nil &Nil::operator=(Nil &&that) noexcept {
  if (this == &that)
    return *this;
  Evaluatable::operator=(that);
  return *this;
}
auto Nil::to_string_impl(const utils::FormatPolicy &format_policy) const
    -> string_type {
  return "nil"s;
}
auto Nil::to_string_view_impl(const utils::FormatPolicy &format_policy) const
    -> string_view_type {
  return "nil"sv;
}
String::String(const string_type &value, const uint_least32_t line)
    : Evaluatable(line), value(value) {}
String::String(const string_view_type value, const uint_least32_t line)
    : Evaluatable(line), value(value) {}
String::String(string_type &&value, const uint_least32_t line) noexcept
    : Evaluatable(line), value(std::move(value)) {}
String::String(const String &that)
    : Evaluatable(that.get_line()), utils::Viewable(that), value(that.value) {}
String::String(String &&that) noexcept
    : Evaluatable(that.get_line()), utils::Viewable(that),
      value(std::move(that.value)) {}
String &String::operator=(const String &that) {
  if (this == &that)
    return *this;
  value = that.value;
  Evaluatable::operator=(that);
  return *this;
}
String &String::operator=(String &&that) noexcept {
  if (this == &that)
    return *this;
  value = std::move(that.value);
  Evaluatable::operator=(that);
  return *this;
}
String &String::operator+(const String &rhs) {
  value += rhs.value;
  return *this;
}
Boolean String::operator==(const String &rhs) const {
  return {value == rhs.value};
}
Boolean String::operator!=(const String &rhs) const {
  return {value != rhs.value};
}
String::operator Boolean() const { return True; }
auto String::to_string_impl(const utils::FormatPolicy &format_policy) const
    -> string_type {
  return value;
}
auto String::to_string_view_impl(const utils::FormatPolicy &format_policy) const
    -> string_view_type {
  return value;
}
Number::Number(const long double value, const uint_least32_t line)
    : Value(line), value(value) {}
Number::Number(const Number &that)
    : Value(that.get_line()), value(that.value) {}
Number::Number(Number &&that) noexcept
    : Value(that.get_line()), value(that.value) {}
Number &Number::operator=(const Number &that) {
  if (this == &that)
    return *this;
  value = that.value;
  Evaluatable::operator=(that);
  return *this;
}
Number &Number::operator=(Number &&that) noexcept {
  if (this == &that)
    return *this;
  value = that.value;
  Evaluatable::operator=(that);
  return *this;
}
Boolean Number::operator==(const Number &that) const {
  return {value == that.value};
}
Boolean Number::operator!=(const Number &that) const {
  return {value != that.value};
}
Boolean Number::operator<(const Number &that) const {
  return {value < that.value};
}
Boolean Number::operator<=(const Number &that) const {
  return {value <= that.value};
}
Boolean Number::operator>(const Number &that) const {
  return {value > that.value};
}
Boolean Number::operator>=(const Number &that) const {
  return {value >= that.value};
}
Number Number::operator-(const Number &that) const {
  return {value - that.value};
}
Number Number::operator+(const Number &that) const {
  return {value + that.value};
}
Number Number::operator*(const Number &that) const {
  return {value * that.value};
}
Number Number::operator/(const Number &that) const {
  return that.value == 0L
             ? Number{std::numeric_limits<long double>::signaling_NaN()}
             : Number{value / that.value};
}
auto Number::to_string_impl(const utils::FormatPolicy &format_policy) const
    -> string_type {
  return utils::format("{}", value);
}
ErrorSyntax::ErrorSyntax(const string_view_type message_sv,
                         const uint_least32_t line)
    : Evaluatable(line),
      message(utils::format("{}\n[line {}]", message_sv, line)) {}
ErrorSyntax::ErrorSyntax(string_type &&message, uint_least32_t &&line) noexcept
    : Evaluatable(line), message(std::move(message)) {}
ErrorSyntax::ErrorSyntax(const ErrorSyntax &that)
    : Evaluatable(that.get_line()), message(that.message) {}
ErrorSyntax::ErrorSyntax(ErrorSyntax &&that) noexcept
    : Evaluatable(that.get_line()), message(std::move(that.message)) {}
ErrorSyntax &ErrorSyntax::operator=(const ErrorSyntax &that) {
  if (this == &that)
    return *this;
  message = that.message;
  Evaluatable::operator=(that);
  return *this;
}
ErrorSyntax &ErrorSyntax::operator=(ErrorSyntax &&that) noexcept {
  if (this == &that)
    return *this;
  message = std::move(that.message);
  Evaluatable::operator=(that);
  return *this;
}
auto ErrorSyntax::to_string_impl(const utils::FormatPolicy &format_policy) const
    -> string_type {
  return message;
}
auto ErrorSyntax::to_string_view_impl(const utils::FormatPolicy &) const
    -> string_view_type {
  return message;
}
} // namespace net::ancillarycat::loxograph::syntax
