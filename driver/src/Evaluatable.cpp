#include <algorithm>
#include <cmath>
#include <memory>
#include <string_view>
#include <utility>

#include "details/IVisitor.hpp"
#include "details/lox_fwd.hpp"

#include "Evaluatable.hpp"
#include "Environment.hpp"
#include "interpreter.hpp"
#include <accat/auxilia/auxilia.hpp>

#include <memory>

namespace accat::lox::evaluation {
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
  is_initializer = that.is_initializer;
}
Function &Function::operator=(Function &&that) noexcept {
  if (this == &that)
    return *this;
  this->my_arity = that.my_arity;
  this->my_function = std::move(that.my_function);
  this->my_env = std::move(that.my_env);
  this->is_initializer = that.is_initializer;
  return *this;
}
Function::Function(const unsigned argc,
                   native_function_t &&func,
                   const env_ptr_t &env,
                   const bool is_initializer) {
  my_arity = argc;
  my_function.emplace(std::move(func));
  my_env = env;
  this->is_initializer = is_initializer;
}

Function::Function(const unsigned argc,
                   custom_function_t &&block,
                   const env_ptr_t &env,
                   const bool is_initializer) {
  my_arity = argc;
  my_function.emplace(std::move(block));
  my_env = env;
  this->is_initializer = is_initializer;
}
Function::Function(const unsigned argc,
                   const function_t &func,
                   const env_ptr_t &env,
                   const bool is_initializer) {
  my_arity = argc;
  my_function = func;
  my_env = env;
  this->is_initializer = is_initializer;
}

auto Function::create_custom(unsigned argc,
                             custom_function_t &&func,
                             const env_ptr_t &env,
                             const bool is_initializer) -> Function {
  return {argc, std::move(func), env, is_initializer};
}

auto Function::create_native(unsigned argc,
                             native_function_t &&func,
                             const env_ptr_t &env,
                             const bool is_initializer) -> Function {
  return {argc, std::move(func), env, is_initializer};
}
auto Function::bind(const Instance &instance) const -> Function {
  auto method_env = Environment::Scope(my_env);
  // TODO: wrong here, copying an instance!!!
  method_env->add("this", {instance});
  return {my_arity, my_function, method_env, is_initializer};
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
              dbg(info, "returning: {}", my_result->to_string())
              return my_result;
            }
            // else, error, return as is
            return res;
          }
        }
        if (is_initializer) {
          dbg(info, "constructor, returning this.")
          return {*my_env->get_at_depth(0, "this")};
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
auto Class::arity() const -> unsigned {
  if (auto init = const_cast<Class *>(this)->get_initializer())
    return init->arity();
  return 0;
}
Class::Class(const std::string_view name,
             const uint_least32_t line,
             methods_t &&methods)
    : Evaluatable(line), name(name), methods(methods) {}
auto Class::call(interpreter &interpreter, args_t &&variants) -> eval_result_t {
  auto instance = Instance{interpreter.get_current_env(), name};
  if (auto initializer = get_initializer())
    if (auto res =
            initializer->bind(instance).call(interpreter, std::move(variants));
        !res)
      return res;

  return {instance};
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
auto Class::get_initializer() -> Function * {
  if (auto it = methods.find("init"); it != methods.end())
    return &it->second;
  return nullptr;
}
Instance::Instance(const env_ptr_t &env, const std::string_view name)
    : class_env(env), class_name(name), fields(std::make_shared<fields_t>()) {}
auto Instance::get_field(const std::string_view name) const -> eval_result_t {
  if (const auto it = fields->find({name.begin(), name.end()});
      it != fields->end())
    return it->second;
  // if field not found, find method
  // clang-format off
  return this
      ->get_class()
      .get_method(name)
      .transform(
        [&](auto &&method) -> IVisitor::variant_type {
          return {method.bind(*this)};
      });
  // clang-format on
}
auto Instance::set_field(const std::string_view name,
                         eval_result_t &&new_val,
                         const bool shallBeDefined) -> auxilia::Status {
  if (!shallBeDefined) {
    // like js or python, we can set a field even if it is not defined yet.
    fields->insert_or_assign({name.begin(), name.end()}, std::move(new_val));
    return {};
  }
  if (auto it = fields->find({name.begin(), name.end()}); it != fields->end()) {
    it->second = std::move(new_val);
    return {};
  }
  return auxilia::NotFoundError(
      "Undefined property '{}'.\n[line {}]", name, get_line());
}
auto Instance::to_string(const auxilia::FormatPolicy &) const -> string_type {
  return get_class().name + " instance";
}
auto Instance::get_class() const -> Class & {
  return (*(class_env->find(class_name)))->second.first.get<Class>();
}
} // namespace accat::lox::evaluation
