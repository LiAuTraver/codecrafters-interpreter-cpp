#include <algorithm>
#include <concepts>
#include <iterator>
#include <map>
#include <ranges>
#include <type_traits>
#include <utility>
#include <memory>
#include <typeinfo>
#include <vector>

#include "loxo_fwd.hpp"

#include "Environment.hpp"
#include "Evaluatable.hpp"
#include "statement.hpp"
#include "expression.hpp"
#include "interpreter.hpp"

namespace net::ancillarycat::loxo {
using utils::match;
using enum TokenType::type_t;
interpreter::interpreter() : env(std::make_shared<Environment>()) {}
auto interpreter::interpret(
    const std::span<std::shared_ptr<statement::Stmt>> stmts) const
    -> stmt_result_t {
  defer { expr_res.clear(); };

  static bool has_init_global_env = false;
  if (!has_init_global_env) {
    auto maybe_env = Environment::createGlobalEnvironment();
    if (!maybe_env.ok())
      return maybe_env.as_status();
    has_init_global_env = true;
    this->env = maybe_env.value();
  }

  for (const auto &stmt : stmts)
    if (auto eval_res = execute(*stmt); !eval_res.ok())
      return eval_res;

  return utils::OkStatus();
}
auto interpreter::save_and_renew_env() const -> const interpreter & {
  prev_env = env;
  env = std::make_shared<env_t>(prev_env);
  return *this;
}
auto interpreter::restore_env() const -> const interpreter & {
  env = prev_env;
  prev_env.reset(); // cvt to nullptr
  return *this;
}
evaluation::Boolean
interpreter::is_true_value(const eval_result_t &value) const {
  return value.visit(match{
      [](const evaluation::Nil &n) {
        return evaluation::Boolean::make_false(n.get_line());
      },
      [](const evaluation::Boolean &b) { return b; },
      [](const auto &) { return evaluation::True; },
  });
}
auto interpreter::is_deep_equal(const eval_result_t &lhs,
                                const eval_result_t &rhs) const
    -> eval_result_t {
  return lhs.index() == rhs.index()
             ? lhs.visit(match{
                   [](const evaluation::Nil &n) -> eval_result_t {
                     return {evaluation::Boolean::make_true(n.get_line())};
                   },
                   [&rhs](const evaluation::Boolean &b) -> eval_result_t {
                     return {utils::get<evaluation::Boolean>(rhs) == b};
                   },
                   [&rhs](const evaluation::String &s) -> eval_result_t {
                     return {utils::get<evaluation::String>(rhs) == s};
                   },
                   [&rhs](const evaluation::Number &n) -> eval_result_t {
                     return {utils::get<evaluation::Number>(rhs) == n};
                   },
                   [](const auto &) -> eval_result_t {
                     return {evaluation::Error{"unimplemented deep equal"sv}};
                   }})
             : evaluation::Boolean{evaluation::False};
}
auto interpreter::get_call_args(const expression::Call &expr) const
    -> std::expected<std::vector<eval_result_t>, eval_result_t> {

  auto args = std::vector<eval_result_t>{};
  args.reserve(expr.args.size());

  // we choose to evaluate argument expressions from left to right, adhering
  // to "Sequenced before" rules in C++11 and later.
  // https://en.cppreference.com/w/cpp/language/eval_order
  for (auto it = expr.args.begin(); it != expr.args.end(); ++it) {
    if (auto res = evaluate(**it); !res.ok())
      return std::unexpected{evaluation::Error{
          utils::format("Error in argument {}",
                        std::ranges::distance(expr.args.begin(), it)),
          expr.paren.line}};
    args.emplace_back(std::move(expr_res));
  }
  return {args};
}
auto interpreter::visit_impl(const statement::Variable &stmt) const
    -> stmt_result_t {
  defer { expr_res.clear(); };

  if (stmt.has_initilizer()) {
    if (auto eval_res = evaluate(*stmt.initializer); !eval_res.ok())
      return eval_res;
    contract_assert(!!std::any_cast<string_view_type>(&stmt.name.literal),
                    1,
                    "variable name should be a string")
    dbg(info,
        "variable name: {}, value: {}",
        std::any_cast<string_view_type>(stmt.name.literal),
        expr_res.underlying_string())
    // string view failed again; not null-terminated
    auto res = env->add(
        stmt.name.to_string(utils::kTokenOnly), expr_res, stmt.name.line);
    return res;
  }
  // if no initializer, it's a nil value.
  return env->add(stmt.name.to_string(utils::kTokenOnly),
                  evaluation::NilValue,
                  stmt.name.line);
}
auto interpreter::visit_impl(const statement::Print &stmt) const
    -> stmt_result_t {
  defer { expr_res.clear(); };

  if (auto eval_res = evaluate(*stmt.value); !eval_res.ok())
    return eval_res;
  stmts_res.emplace_back(expr_res);
  return utils::OkStatus();
}
auto interpreter::visit_impl(const statement::If &stmt) const -> stmt_result_t {
  defer { expr_res.clear(); };

  if (auto eval_res = evaluate(*stmt.condition); !eval_res.ok())
    return eval_res;
  if (is_true_value(expr_res).is_true()) {
    if (auto eval_res = execute(*stmt.then_branch); !eval_res.ok())
      return eval_res;
    return utils::OkStatus();
  }
  // else we execute the else branch.
  if (stmt.else_branch) { // maybe we dont have an else branch, so check it.
    if (auto eval_res = execute(*stmt.else_branch); !eval_res.ok())
      return eval_res;
    return utils::OkStatus();
  }
  // no else branch do nothing. BUT don't forget to clear the expr_res!
  return utils::OkStatus();
}
auto interpreter::visit_impl(const statement::While &stmt) const
    -> stmt_result_t {
  defer { expr_res.clear(); };

  do {
    if (auto res = evaluate(*stmt.condition); !res.ok())
      return res;
    if (not is_true_value(expr_res).is_true())
      break;
    if (auto res = execute(*stmt.body); !res.ok())
      return res;
  } while (true);

  return utils::OkStatus();
}
auto interpreter::visit_impl(const statement::For &stmt) const
    -> stmt_result_t {
  defer { expr_res.clear(); };

  if (stmt.initializer)
    if (auto res = execute(*stmt.initializer); !res.ok())
      return res;

  while (true) {
    if (stmt.condition) {
      if (auto res = evaluate(*stmt.condition); !res.ok())
        return res;
      if (not is_true_value(expr_res).is_true())
        break;
    }
    if (auto res = execute(*stmt.body); !res.ok())
      return res;
    if (stmt.increment) {
      if (auto res = evaluate(*stmt.increment); !res.ok())
        return res;
    }
  }
  return utils::OkStatus();
}
auto interpreter::visit_impl(const statement::Function &stmt) const
    -> stmt_result_t {
  defer { expr_res.clear(); };
  // TODO: function overloading
  if (auto res = env->get(stmt.name.to_string(utils::kTokenOnly));
      !res.empty()) {
    dbg(warn, "function overloading is not supported yet.");
    return utils::InvalidArgument(
        utils::format("Function '{}' already defined.",
                      stmt.name.to_string(utils::kTokenOnly)));
  }
  return env->add(
      stmt.name.to_string(utils::kTokenOnly),
      evaluation::Callable::create_custom(
          stmt.parameters.size(),
          std::move(std::remove_const_t<statement::Function>(stmt))),
      stmt.name.line);
}
auto interpreter::visit_impl(const statement::IllegalStmt &stmt) const
    -> stmt_result_t {
  return utils::InvalidArgument(stmt.message);
}
auto interpreter::visit_impl(const statement::Expression &stmt) const
    -> stmt_result_t {
  return evaluate(*stmt.expr);
}
auto interpreter::visit_impl(const statement::Block &stmt) const
    -> stmt_result_t {
  auto original_env = env; // save the original environment
  auto sub_env = std::make_shared<Environment>(env);
  env = sub_env;
  for (const auto &scoped_stmt : stmt.statements) {
    if (auto eval_res = execute(*scoped_stmt); !eval_res.ok()) {
      env = original_env; // restore
      return eval_res;
    }
  }
  env = original_env; // restore
  return utils::OkStatus();
}
auto interpreter::execute_impl(const statement::Stmt &stmt) const
    -> stmt_result_t {
  return stmt.accept(*this);
}
auto interpreter::get_result_impl() const -> eval_result_t { return expr_res; }

auto interpreter::evaluate_impl(const expression::Expr &expr) const
    -> stmt_result_t {
  return expr_res.set(expr.accept(*this))
      .visit(match{[](const evaluation::Error &e) {
                     return utils::InvalidArgument(e.to_string_view());
                   },
                   [](const utils::Monostate &) {
                     return utils::EmptyInput("no expr was evaluated.");
                   },
                   [](const auto &) { return utils::OkStatus(); }});
}
auto interpreter::visit_impl(const statement::Return &expr) const
    -> stmt_result_t {
  if (!this->prev_env) {
    return utils::InvalidArgument("return statement outside of function.");
  }
  // TODO()
}
auto interpreter::visit_impl(const expression::Literal &expr) const
    -> eval_result_t {
  dbg(info, "literal type: {}", expr.literal.type);
  if (expr.literal.is_type(kMonostate)) {
    dbg(critical, "should not happen.");
    contract_assert(false)
    return {};
  }
  if (expr.literal.is_type(kNil)) {
    return evaluation::Nil{expr.literal.line};
  }
  if (expr.literal.is_type(kTrue)) {
    return evaluation::Boolean{true, expr.literal.line};
  }
  if (expr.literal.is_type(kFalse)) {
    return evaluation::Boolean{false, expr.literal.line};
  }
  if (expr.literal.is_type(kString)) {
    return evaluation::String{
        std::any_cast<string_view_type>(expr.literal.literal),
        expr.literal.line};
  }
  if (expr.literal.is_type(kNumber)) {
    return evaluation::Number{std::any_cast<long double>(expr.literal.literal),
                              expr.literal.line};
  }
  // temporary solution.
  return evaluation::Error{"Expected literal value"s, expr.literal.line};
}

auto interpreter::visit_impl(const expression::Unary &expr) const
    -> eval_result_t {
  auto inner_expr = expr.expr->accept(*this);
  if (expr.op.is_type(kMinus)) {
    if (utils::holds_alternative<evaluation::Number>(inner_expr)) {
      auto value = utils::get<evaluation::Number>(inner_expr);
      dbg(trace, "unary minus: {}", value);
      return evaluation::Number{value * (-1)};
    }
    return evaluation::Error{"Operand must be a number."s, expr.op.line};
  }
  if (expr.op.is_type(kBang)) {
    auto value = is_true_value(inner_expr);
    dbg(trace, "unary bang: {}", value);
    return evaluation::Boolean{!value};
  }
  contract_assert(false, 1, "unreachable code reached");
  return {utils::Monostate{}};
}

auto interpreter::visit_impl(const expression::Binary &expr) const
    -> eval_result_t {
  auto lhs = expr.left->accept(*this);
  auto rhs = expr.right->accept(*this);
  if (expr.op.is_type(kEqualEqual)) {
    return is_deep_equal(lhs, rhs);
  }
  if (expr.op.is_type(kBangEqual)) {
    auto result = is_deep_equal(lhs, rhs);
    if (auto ptr = utils::get_if<evaluation::Boolean>(&result))
      return {!*ptr};
    return result;
  }
  if (auto ptr = utils::get_if<evaluation::Error>(&lhs))
    return {*ptr};

  if (auto ptr = utils::get_if<evaluation::Error>(&rhs))
    return {*ptr};

  if (lhs.index() != rhs.index()) {
    dbg(error, "type mismatch: lhs: {}, rhs: {}", lhs.index(), rhs.index());
    dbg(warn, "current implementation only support same type binary operation");
    return evaluation::Error{"Operands must be two numbers or two strings."s,
                             expr.op.line};
  }
  if (utils::holds_alternative<evaluation::String>(lhs)) {
    if (expr.op.is_type(kPlus)) {
      return evaluation::String{utils::get<evaluation::String>(lhs) +
                                utils::get<evaluation::String>(rhs)};
    }
  }
  if (utils::holds_alternative<evaluation::Number>(lhs)) {
    auto real_lhs = utils::get<evaluation::Number>(lhs);
    auto real_rhs = utils::get<evaluation::Number>(rhs);
    switch (expr.op.type.type) {
    case kMinus:
      return {evaluation::Number{real_lhs - real_rhs}};
    case kPlus:
      return {evaluation::Number{real_lhs + real_rhs}};
    case kSlash:
      return {evaluation::Number{real_lhs / real_rhs}};
    case kStar:
      return {evaluation::Number{real_lhs * real_rhs}};
    case kGreater:
      return {evaluation::Boolean{real_lhs > real_rhs}};
    case kGreaterEqual:
      return {evaluation::Boolean{real_lhs >= real_rhs}};
    case kLess:
      return {evaluation::Boolean{real_lhs < real_rhs}};
    case kLessEqual:
      return {evaluation::Boolean{real_lhs <= real_rhs}};
    default:
      break;
    }
  }
  dbg(error, "unimplemented binary operator: {}", expr.op.to_string());
  contract_assert(false);
  return evaluation::Error{"unimplemented binary operator"s, expr.op.line};
}
auto interpreter::visit_impl(const expression::Grouping &expr) const
    -> eval_result_t {
  return {expr.expr->accept(*this)};
}
auto interpreter::visit_impl(const expression::Variable &expr) const
    -> eval_result_t {
  if (auto res = env->get(expr.name.to_string(utils::FormatPolicy::kTokenOnly));
      !res.empty())
    return res;

  return {evaluation::Error{
      utils::format("Undefined variable '{}'.",
                    expr.name.to_string(utils::FormatPolicy::kTokenOnly)),
      expr.name.line}};
}
auto interpreter::visit_impl(const expression::Assignment &expr) const
    -> eval_result_t {
  // FIXME: here my logic went away. fixme here.
  if (!this->evaluate(*expr.value_expr).ok())
    return {evaluation::Error{"Error in assignment"s, expr.name.line}};

  if (env->reassign(expr.name.to_string(utils::FormatPolicy::kTokenOnly),
                    expr_res,
                    expr.name.line)
          .ok())
    return expr_res;

  return {evaluation::Error{
      utils::format("Undefined variable '{}'.",
                    expr.name.to_string(utils::FormatPolicy::kTokenOnly)),
      expr.name.line}};
}
auto interpreter::visit_impl(const expression::Logical &expr) const
    -> eval_result_t {
  return expr.left->accept(*this).visit(
      match{[](const utils::Monostate &) -> eval_result_t {
              contract_assert(false, 1, "should not happen");
              return {evaluation::Error{"Error in logical expression"s, 0}};
            },
            [](const evaluation::Error &e) -> eval_result_t {
              dbg(error, "error in logical expression: {}", e.to_string());
              return {e};
            },
            [this, &expr](const auto &lhs) -> eval_result_t {
              if (is_true_value(lhs).is_true()) {
                if (expr.op.is_type(kOr))
                  return {lhs};
                if (expr.op.is_type(kAnd))
                  return {expr.right->accept(*this)};
                contract_assert(false, 1, "unimplemented logical operator");
                return {utils::Monostate{}};
              }
              // left is false, evaluate right.
              if (expr.op.is_type(kOr))
                return {expr.right->accept(*this)};
              if (expr.op.is_type(kAnd))
                return {evaluation::Boolean{false, expr.op.line}};
              contract_assert(false, 1, "unimplemented logical operator");
              return {utils::Monostate{}};
            }});
}
auto interpreter::visit_impl(const expression::Call &expr) const
    -> eval_result_t {
  defer {
    expr_res.clear();
  }; // FIXME: should not call `clear` when evaluating an expression.

  if (auto res = evaluate(*expr.callee); !res.ok())
    return {evaluation::Error{res.message()}};

  // `expr_res` would change in `get_call_args`, so we need to save it.
  const auto callee = expr.callee;
  if (!utils::holds_alternative<evaluation::Callable>(expr_res)) {
    return {evaluation::Error{
        utils::format("Not a function: {}", callee->to_string()),
        expr.paren.line}};
  }
  auto callable = utils::get<evaluation::Callable>(expr_res);

  auto maybe_args = get_call_args(expr);
  if (!maybe_args.has_value())
    return {maybe_args.error()};
  if (maybe_args->size() == callable.arity())
    return (callable).call(*this, *maybe_args);

  return evaluation::Error{
      maybe_args->size() > callable.arity()
          ? utils::format("Too many arguments to call function '{}': "
                          "expected {} but got {}",
                          callee->to_string(),
                          callable.arity(),
                          maybe_args->size())
          : utils::format("Too few arguments to call function '{}': "
                          "expected {} but got {}",
                          callee->to_string(),
                          callable.arity(),
                          maybe_args->size()),
      expr.paren.line};
}
auto interpreter::visit_impl(const expression::IllegalExpr &expr) const
    -> eval_result_t {
  return {
      evaluation::Error{utils::format("Illegal expression: {}",
                                      expr.token.to_string(utils::kTokenOnly)),
                        expr.token.line}};
}

auto interpreter::expr_to_string(const utils::FormatPolicy &format_policy) const
    -> string_type {
  return value_to_string(format_policy, expr_res);
}
auto interpreter::value_to_string(const utils::FormatPolicy &format_policy,
                                  const eval_result_t &value) const
    -> string_type {
  return value.underlying_string(format_policy);
}
auto interpreter::to_string_impl(const utils::FormatPolicy &format_policy) const
    -> string_type {
  dbg(info, "expr_res index: {}", expr_res.index());
  dbg(info, "stmts size: {}", stmts_res.size());
  if (stmts_res.empty())
    // we are parse an expression, not a statement
    return value_to_string(format_policy, expr_res);

  contract_assert(expr_res.empty(), 1, "expr_res should be empty")
  string_type result_str;
  // clang-format off
  std::ranges::for_each(
      stmts_res 
      | std::ranges::views::transform([&](const auto &res) {
        return value_to_string(format_policy, res);
      }) 
      | std::ranges::views::filter([](const auto &str) {
        // FIXME: temporary solution: skip newline if the result is empty(i.e.,Monostate)
        return !str.empty();
      }),
      [&](const auto &str) { 
        result_str += std::move(str) + '\n'; 
  });
  // clang-format on
  return result_str;
}
LOXO_API void delete_interpreter_fwd(interpreter *ptr) { delete ptr; }
} // namespace net::ancillarycat::loxo
