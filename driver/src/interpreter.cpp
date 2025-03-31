#include <algorithm>
#include <concepts>
#include <expected>
#include <iterator>
#include <map>
#include <ranges>
#include <type_traits>
#include <utility>
#include <memory>
#include <typeinfo>
#include <vector>

#include <accat/auxilia/auxilia.hpp>

#include "Token.hpp"
#include "details/loxo_fwd.hpp"
#include "Environment.hpp"
#include "Evaluatable.hpp"
#include "statement.hpp"
#include "expression.hpp"
#include "interpreter.hpp"

namespace accat::loxo {
using auxilia::match;
using enum TokenType::type_t;
using enum auxilia::FormatPolicy;
interpreter::interpreter() : env(std::make_shared<Environment>()) {}
auto interpreter::interpret(
    const std::span<std::shared_ptr<statement::Stmt>> stmts) const
    -> eval_result_t {
  is_interpreting_stmts = true;
  static bool has_init_global_env = false;
  if (!has_init_global_env) {
    this->env = Environment::Global();
    has_init_global_env = true;
  }

  for (const auto &stmt : stmts)
    if (auto eval_res = execute(*stmt); !eval_res) {
      last_expr_res->clear();
      return eval_res;
    }

  return {};
}

auto interpreter::set_env(const env_ptr_t &new_env) const
    -> const interpreter & {
  env = new_env;
  return *this;
}

evaluation::Boolean
interpreter::is_true_value(const eval_result_t &value) const {
  return value->visit(match{
      [](const evaluation::Nil &n) {
        return evaluation::Boolean::make_false(n.get_line());
      },
      [](const evaluation::Boolean &b) { return b; },
      [](const auto &) { return evaluation::True; },
  });
}
auto interpreter::is_deep_equal(const eval_result_t &lhs,
                                const eval_result_t &rhs) const
    -> evaluation::Boolean {
  using Bool = evaluation::Boolean;
  auto pattern = match{
      [](const evaluation::Nil &lhs, const evaluation::Nil &rhs) -> Bool {
        return {Bool::make_true(lhs.get_line())};
      },
      []<typename T>(const T &lhs, const T &rhs) -> Bool {
        return {lhs == rhs};
      },
      [](const auto &lhs, const auto &rhs) -> Bool {
        return {Bool{evaluation::False}};
      },
  };
  return auxilia::visit(pattern, *lhs, *rhs);
}
auto interpreter::get_call_args(const expression::Call &expr) const
    -> auxilia::StatusOr<std::vector<variant_type>> {
  auto args = std::vector<variant_type>{};
  args.reserve(expr.args.size());

  // we choose to evaluate argument expressions from left to right, adhering
  // to "Sequenced before" rules in C++11 and later.
  // https://en.cppreference.com/w/cpp/language/eval_order
  for (const auto &arg : expr.args) {
    auto res = evaluate(*arg);
    if (!res)
      return {res};
    args.emplace_back(*res);
  }
  return {args};
}
auto interpreter::visit_impl(const statement::Variable &stmt) const
    -> eval_result_t {

  if (stmt.has_initilizer()) {
    auto eval_res = evaluate(*stmt.initializer);
    if (!eval_res)
      return eval_res;
    contract_assert(stmt.name.literal.is_type<string_view_type>(),
                    "variable name should be a string")
    dbg(trace,
        "variable name: {}, value: {}",
        stmt.name.literal.get<string_view_type>(),
        eval_res->underlying_string())
    // string view failed again; not null-terminated
    return {env->add(
        stmt.name.to_string(kDetailed), eval_res.value(), stmt.name.line)};
  }
  // if no initializer, it's a nil value.
  return env->add(
      stmt.name.to_string(kDetailed), evaluation::NilValue, stmt.name.line);
}
auto interpreter::visit_impl(const statement::Print &stmt) const
    -> eval_result_t {
  auto eval_res = evaluate(*stmt.value);
  if (!eval_res)
    return eval_res;
  stmts_res.emplace_back(eval_res.value());
  return {{evaluation::NilValue}};
}
auto interpreter::visit_impl(const statement::If &stmt) const -> eval_result_t {
  auto eval_res = evaluate(*stmt.condition);
  if (!eval_res)
    return eval_res;
  if (is_true_value(eval_res.value()).is_true()) {
    return execute(*stmt.then_branch);
  }
  // else we execute the else branch.
  if (stmt.else_branch) { // maybe we dont have an else branch, so check it.
    return execute(*stmt.else_branch);
  }
  return {eval_res.value()};
}
auto interpreter::visit_impl(const statement::While &stmt) const
    -> eval_result_t {
  eval_result_t res;
  do {
    auto eval_res = evaluate(*stmt.condition);
    if (!eval_res)
      return eval_res;
    if (not is_true_value(eval_res.value()).is_true())
      break;
    res = execute(*stmt.body);
    if (!res)
      return res;
  } while (true);

  return {*res};
}
auto interpreter::visit_impl(const statement::For &stmt) const
    -> eval_result_t {

  if (stmt.initializer)
    if (auto res = execute(*stmt.initializer); !res)
      return res;

  while (true) {
    if (stmt.condition) {
      auto cond_res = evaluate(*stmt.condition);
      if (!cond_res)
        return cond_res;
      if (not is_true_value(*cond_res).is_true())
        break;
    }
    if (auto res = execute(*stmt.body); !res)
      return res;
    if (stmt.increment) {
      if (auto res = evaluate(*stmt.increment); !res)
        return res;
    }
  }
  return {};
}
auto interpreter::visit_impl(const statement::Function &stmt) const
    -> eval_result_t {
  // TODO: function overloading
  if (auto res = env->get(stmt.name.to_string(kDetailed)); !res.empty()) {
    if (!res.is_type<evaluation::Callable>()) {
      dbg(error,
          "bad function definition: {} is not a function",
          stmt.name.to_string(kDefault))
      return res;
    }
    // if arity is same, warn and overwrite the function;
    // if arity is different, just as overloading.
    auto callable = res.get<evaluation::Callable>();
    if (callable.arity() == stmt.parameters.size()) {
      dbg(warn, "function {} already defined", stmt.name.to_string(kDetailed))
    } else {
      dbg(info, "overloading function: {}", stmt.name.to_string(kDetailed))
      TODO(unordered map, overloading)
    }
  }

  dbg(info, "func name: {}", stmt.name.to_string(kDetailed))

  // clang-format off
  auto callable = evaluation::Callable::create_custom(
          stmt.parameters.size(),
          {stmt.name.to_string(kDetailed),
           stmt.parameters
           | std::ranges::views::transform([&](const auto &param) {
               return param.to_string(kDetailed);
             })
           | std::ranges::to<std::vector<string_type>>(),
           stmt.body.statements},
           this->env); 
  return env->add(
      stmt.name.to_string(kDetailed),
      callable,
      stmt.name.line);
  // clang-format on
}
auto interpreter::visit_impl(const statement::Expression &stmt) const
    -> eval_result_t {
  return evaluate(*stmt.expr);
}
auto interpreter::visit_impl(const statement::Block &stmt) const
    -> eval_result_t {
  auto original_env = env; // save the original environment
  auto sub_env = std::make_shared<Environment>(env);
  env = sub_env;
  for (const auto &scoped_stmt : stmt.statements) {
    if (auto eval_res = execute(*scoped_stmt); !eval_res) {
      env = original_env; // restore // TODO: shall i also restore env when
                          // eval_res cantains `returning`?
      return eval_res;
    }
  }
  env = original_env; // restore
  return {};
}
auto interpreter::execute_impl(const statement::Stmt &stmt) const
    -> eval_result_t {
  return stmt.accept(*this);
}
auto interpreter::get_result_impl() const -> eval_result_t {
  return last_expr_res;
}

auto interpreter::evaluate_impl(const expression::Expr &expr) const
    -> eval_result_t {
  auto res = expr.accept(*this);
  if (!res)
    return res;
  dbg(info, "result: {}", res->underlying_string())
  return last_expr_res.reset(*std::move(res));
}
auto interpreter::visit_impl(const statement::Return &expr) const
    -> eval_result_t {
  if (this->env == Environment::Global()) {
    return {
        auxilia::InvalidArgumentError("Cannot return from top-level code.")};
  }

  if (not expr.value) {
    dbg(info, "returning nil")
    return Returning({{evaluation::NilValue}});
  }
  auto res = evaluate(*expr.value);
  dbg(info, "return value: {}", res->underlying_string())
  if (!res) {
    return res;
  }
  dbg(trace, "result: {}", res->underlying_string())
  return Returning(*res);
}
auto interpreter::visit_impl(const expression::Literal &expr) const
    -> eval_result_t {
  dbg(trace, "literal type: {}", expr.literal.type)

  auto T = [&expr](TokenType::type_t t) constexpr noexcept -> bool {
    return expr.literal.is_type(t);
  };

  if (T(kMonostate)) {
    dbg_break
  }
  if (T(kNil)) {
    return {evaluation::Nil{expr.literal.line}};
  }
  if (T(kTrue)) {
    return {evaluation::Boolean{true, expr.literal.line}};
  }
  if (T(kFalse)) {
    return {evaluation::Boolean{false, expr.literal.line}};
  }
  if (T(kString)) {
    return {evaluation::String{expr.literal.literal.get<string_view_type>(),
                               expr.literal.line}};
  }
  if (T(kNumber)) {
    return {evaluation::Number{expr.literal.literal.get<long double>(),
                               expr.literal.line}};
  }
  return {auxilia::InvalidArgumentError("Expected literal value.\n[line {}]",
                                        expr.literal.line)};
}

auto interpreter::visit_impl(const expression::Unary &expr) const
    -> eval_result_t {
  auto inner_expr = expr.expr->accept(*this);
  if (expr.op.is_type(kMinus)) {
    if (inner_expr->is_type<evaluation::Number>()) {
      auto value = inner_expr->get<evaluation::Number>();
      dbg(trace, "unary minus: {}", value)
      return {evaluation::Number{value * (-1)}};
    }
    return {auxilia::InvalidArgumentError(
        "Operand must be a number.\n[line {}]", expr.op.line)};
  }
  if (expr.op.is_type(kBang)) {
    auto value = is_true_value(*inner_expr);
    dbg(trace, "unary bang: {}", value.to_string(kDefault))
    return {evaluation::Boolean{!value}};
  }
  contract_assert(false, "unreachable code reached")
  return {};
}

auto interpreter::visit_impl(const expression::Binary &expr) const
    -> eval_result_t {
  auto lhs = expr.left->accept(*this);
  if (!lhs) {
    return lhs;
  }

  auto rhs = expr.right->accept(*this);
  if (!rhs) {
    return rhs;
  }

  if (expr.op.is_type(kEqualEqual)) {
    return {{is_deep_equal(lhs, rhs)}};
  }
  if (expr.op.is_type(kBangEqual)) {
    return {{!is_deep_equal(lhs, rhs)}};
  }

  if (lhs->index() != rhs->index()) {
    dbg(error, "type mismatch: lhs: {}, rhs: {}", lhs->index(), rhs->index())
    dbg(warn, "current implementation only support same type binary operation")
    return {auxilia::InvalidArgumentError(
        "Operands must be two numbers or two strings.\n[line "
        "{}]",
        expr.op.line)};
  }
  if (lhs->is_type<evaluation::String>()) {
    if (expr.op.is_type(kPlus)) {
      auto str_lhs = lhs->get<evaluation::String>();
      auto str_rhs = rhs->get<evaluation::String>();
      return {evaluation::String{str_lhs + str_rhs}};
    }
  }
  if (lhs->is_type<evaluation::Number>()) {
    auto real_lhs = lhs->get<evaluation::Number>();
    auto real_rhs = rhs->get<evaluation::Number>();
    switch (expr.op.type.type) {
    case kMinus:
      return {{real_lhs - real_rhs}};
    case kPlus:
      return {{real_lhs + real_rhs}};
    case kSlash:
      return {{real_lhs / real_rhs}};
    case kStar:
      return {{real_lhs * real_rhs}};
    case kGreater:
      return {{real_lhs > real_rhs}};
    case kGreaterEqual:
      return {{real_lhs >= real_rhs}};
    case kLess:
      return {{real_lhs < real_rhs}};
    case kLessEqual:
      return {{real_lhs <= real_rhs}};
    default:
      break;
    }
  }
  dbg(error, "unimplemented binary operator: {}", expr.op.to_string())
  dbg_break
  return {auxilia::InvalidArgumentError(
      "unimplemented binary operator.\n[line {}]", expr.op.line)};
}
auto interpreter::visit_impl(const expression::Grouping &expr) const
    -> eval_result_t {
  return {expr.expr->accept(*this)};
}
auto interpreter::visit_impl(const expression::Variable &expr) const
    -> eval_result_t {
  if (auto res = env->get(expr.name.to_string(kDetailed)); !res.empty())
    return res;

  return {auxilia::NotFoundError("Undefined variable '{}'.\n[line {}]",
                                 expr.name.to_string(kDetailed),
                                 expr.name.line)};
}
auto interpreter::visit_impl(const expression::Assignment &expr) const
    -> eval_result_t {
  // FIXME: here my logic went away. fixme here.
  auto res = this->evaluate(*expr.value_expr);
  if (!res)
    return res;

  if (!env->reassign(expr.name.to_string(kDetailed), *res, expr.name.line))
    return {auxilia::NotFoundError("Undefined variable '{}'.\n[line {}]",
                                   expr.name.to_string(kDetailed),
                                   expr.name.line)};
  return *res;
}
auto interpreter::visit_impl(const expression::Logical &expr) const
    -> eval_result_t {
  auto lhs = expr.left->accept(*this);
  if (!lhs)
    return lhs;
  if (is_true_value(*lhs).is_true()) {
    if (expr.op.is_type(kOr))
      return {*lhs};
    if (expr.op.is_type(kAnd))
      return {expr.right->accept(*this)};
    contract_assert(false, "unimplemented logical operator")
    return {auxilia::Monostate{}};
  }
  // left is false, evaluate right.
  if (expr.op.is_type(kOr))
    return {expr.right->accept(*this)};
  if (expr.op.is_type(kAnd))
    return {evaluation::Boolean{false, expr.op.line}};
  contract_assert(false, "unimplemented logical operator")
  return {auxilia::Monostate{}};
}
auto interpreter::visit_impl(const expression::Call &expr) const
    -> eval_result_t {
  auto res = evaluate(*expr.callee);
  if (!res)
    return res;

  // `result` would change in `get_call_args`, so we need to save it.
  const auto callee = expr.callee;
  if (!res->is_type<evaluation::Callable>()) {
    dbg(error,
        "bad function call: {} is not a function",
        callee->to_string(kDefault))
    return {auxilia::NotFoundError(
        "Can only call functions and classes.\n[line {}]", expr.paren.line)};
  }

  auto callable = res->get<evaluation::Callable>();
  auto maybe_args = get_call_args(expr);

  if (!maybe_args)
    return {maybe_args.as_status()};
  auto args = *std::move(maybe_args);

  if (args.size() == callable.arity())
    // clear `Returning` status has already been implemented in `call` method.
    // just return here.
    return callable.call(*this, std::move(args));

  return {auxilia::InvalidArgumentError(
      "Too {} arguments to call function '{}': "
      "expected {} but got {}",
      args.size() > callable.arity() ? "many" : "few",
      callee->to_string(kDefault),
      callable.arity(),
      args.size())};
}

auto interpreter::expr_to_string(
    const auxilia::FormatPolicy &format_policy) const -> string_type {
  return value_to_string(format_policy, last_expr_res);
}
auto interpreter::value_to_string(const auxilia::FormatPolicy &format_policy,
                                  const eval_result_t &value) const
    -> string_type {
  return value->underlying_string(format_policy);
}
auto interpreter::to_string(const auxilia::FormatPolicy &format_policy) const
    -> string_type {
  dbg(info, "last_expr_res index: {}", last_expr_res->index())
  dbg(info, "stmts size: {}", stmts_res.size())
  if (stmts_res.empty()) { // we are parse an expression, not a statement
    if (last_expr_res->index() && !is_interpreting_stmts)
      return value_to_string(format_policy, last_expr_res);
    return {};
  }

  string_type result_str;
  // clang-format off
  std::ranges::for_each(
      stmts_res 
      | std::ranges::views::transform([&](const auto &res) {
        return value_to_string(format_policy, res);
      }) 
      | std::ranges::views::filter([](const auto &str) {
        // FIXME: temporary solution: skip newline if the result is empty(i.e., Monostate)
        return !str.empty();
      }),
      [&](const auto &str) { 
        result_str += std::move(str) + '\n'; 
  });
  // clang-format on
  return result_str;
}
LOXO_API void delete_interpreter_fwd(interpreter *ptr) { delete ptr; }
} // namespace accat::loxo
