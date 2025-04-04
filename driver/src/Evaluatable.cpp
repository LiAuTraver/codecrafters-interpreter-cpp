#include <algorithm>
#include <cmath>
#include <memory>
#include <utility>

#include "details/loxo_fwd.hpp"

#include "Evaluatable.hpp"
#include "Environment.hpp"
#include "interpreter.hpp"
#include <accat/auxilia/auxilia.hpp>

#include <memory>

namespace accat::loxo::evaluation {
using auxilia::match;

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

auto Boolean::make_true(const uint_least32_t line) -> Boolean {
  return Boolean{true, line};
}

auto Boolean::make_false(const uint_least32_t line) -> Boolean {
  return Boolean{false, line};
}

bool Boolean::is_true() const noexcept { return value; }

auto Boolean::to_string(const auxilia::FormatPolicy &format_policy) const
    -> string_type {
  return value ? "true" : "false";
}

auto Boolean::to_string_view(const auxilia::FormatPolicy &format_policy) const
    -> string_view_type {
  return value ? "true"sv : "false"sv;
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

auto Nil::to_string(const auxilia::FormatPolicy &format_policy) const
    -> string_type {
  return "nil"s;
}

auto Nil::to_string_view(const auxilia::FormatPolicy &format_policy) const
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
    : Evaluatable(that.get_line()), value(that.value) {}

String::String(String &&that) noexcept
    : Evaluatable(that.get_line()), value(std::move(that.value)) {}

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

String String::operator+(const String &rhs) const {
  return String{value + rhs.value};
}

Boolean String::operator==(const String &rhs) const {
  return {value == rhs.value};
}

Boolean String::operator!=(const String &rhs) const {
  return {value != rhs.value};
}

String::operator Boolean() const { return True; }

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

Number &Number::operator+=(const Number &that) {
  value += that.value;
  return *this;
}

Number &Number::operator-=(const Number &that) {
  value -= that.value;
  return *this;
}

Number &Number::operator*=(const Number &that) {
  value *= that.value;
  return *this;
}

Number &Number::operator/=(const Number &that) {
  value = that.value == 0L ? std::numeric_limits<long double>::signaling_NaN()
                           : value / that.value;
  return *this;
}

auto Number::to_string(const auxilia::FormatPolicy &format_policy) const
    -> string_type {
  return auxilia::format("{}", value);
}

Function::Function(Function &&that) noexcept {
  my_arity = that.my_arity;
  my_function = std::move(that.my_function);
  my_env = std::move(that.my_env);
}
Function &Function::operator=(Function &&that) noexcept {
  if (this == &that)
    return *this;
  this->my_arity = that.my_arity;
  this->my_function = std::move(that.my_function);
  this->my_env = std::move(that.my_env);
  return *this;
}
Function::Function(const unsigned argc,
                   native_function_t &&func,
                   const env_ptr_t &env) {
  my_arity = argc;
  my_function.emplace(std::move(func));
  my_env = env;
}

Function::Function(const unsigned argc,
                   custom_function_t &&block,
                   const env_ptr_t &env) {
  my_arity = argc;
  my_function.emplace(std::move(block));
  my_env = env;
}

auto Function::create_custom(unsigned argc,
                             custom_function_t &&func,
                             const env_ptr_t &env) -> Function {
  return {argc, std::move(func), env};
}

auto Function::create_native(unsigned argc,
                             native_function_t &&func,
                             const env_ptr_t &env) -> Function {
  return {argc, std::move(func), env};
}

auto Function::call(interpreter &interpreter, args_t &&args) -> eval_result_t {
  precondition(this->arity() == args.size(),
               "arity mismatch; should check it before calling")
  return my_function.visit(match(
      [&](const native_function_t &native_function) -> eval_result_t {
        return {native_function.operator()(interpreter, args)};
      },
      [&](const custom_function_t &custom_function) -> eval_result_t {
        auto saved_env = interpreter.get_current_env();

        auto scoped_env = Environment::Scope(this->my_env);

        for (size_t i = 0; i < custom_function.parameters.size(); ++i) {
          if (auto res =
                  scoped_env->add(custom_function.parameters[i], args[i]);
              !res.ok()) {
            return {res};
          }
        }

        dbg(info, "entering a function...")
        interpreter.set_env(scoped_env);

        defer { interpreter.set_env(saved_env); };

        for (const auto &index : custom_function.body) {
          if (auto res = interpreter.execute(*index); !res) {
            if (res.is_return()) {
              auto my_result = interpreter.get_result();
              // FIXME: i my logic was completely gone here: `last_expr`
              //              itself was a mistake!
              dbg(info, "returning: {}", my_result->underlying_string())
              return my_result;
            }
            // else, error, return as is
            return res;
          }
        }

        dbg(info, "void function, returning nil.")
        return {{NilValue}};
      },
      [](const auto &) -> eval_result_t {
        dbg_break
        return {auxilia::NotFoundError("no function to call")};
      }));
}

auto Function::to_string(const auxilia::FormatPolicy &) const -> string_type {
  return my_function.visit(match{
      [](const native_function_t &) { return "<native fn>"s; },
      [](const custom_function_t &f) {
        return auxilia::format("<fn {}>", f.name);
      },
      [](const auto &) { return "<unknown fn>"s; },
  });
}
auto Class::call(interpreter &interpreter, args_t &&variants) -> eval_result_t {
  return {Instance{this->name}};
}
auto Class::get_method(const std::string_view name) const
    -> auxilia::StatusOr<Function> {
  if (const auto it = methods.find({name.begin(), name.end()});
      it != methods.end())
    return {it->second};

  return auxilia::NotFoundError(
      "Undefined property '{}'.\n[line {}]", name, get_line());
}
auto Class::to_string(const auxilia::FormatPolicy &) const -> string_type {
  return name;
}
auto Instance::get_field(const std::string_view name) const -> eval_result_t {
  if (const auto it = fields.find({name.begin(), name.end()});
      it != fields.end())
    return it->second;
  // if field not found, find method
  // TODO
  // my_calss.get_methods
  return auxilia::NotFoundError(
      "Undefined property '{}'.\n[line {}]", name, get_line());
}
auto Instance::set_field(std::string_view name, eval_result_t &&new_val)
    -> auxilia::Status {
  if (auto it = fields.find({name.begin(), name.end()}); it != fields.end()) {
    it->second = std::move(new_val);
    return {};
  }
  return {auxilia::NotFoundError(
      "Undefined property '{}'.\n[line {}]", name, get_line())};
}
auto Instance::to_string(const auxilia::FormatPolicy &) const -> string_type {
  return class_name + " instance";
}
} // namespace accat::loxo::evaluation
