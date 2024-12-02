#include <concepts>

#include <map>
#include <utility>
#include <memory>
#include <typeinfo>

#include "Evaluatable.hpp"
#include "Monostate.hpp"
#include "TokenType.hpp"
#include "config.hpp"
#include "statement.hpp"
#include "status.hpp"
#include "utils.hpp"
#include "loxo_fwd.hpp"
#include "expression.hpp"
#include "interpreter.hpp"
#include "Variant.hpp"

namespace net::ancillarycat::loxograph {
using utils::match;
using enum TokenType::type_t;
utils::Status interpreter::interpret(
    const std::span<std::shared_ptr<statement::Stmt>> stmts) const {
  defer { expr_res.clear(); };

  for (const auto &stmt : stmts)
    if (auto eval_res = execute(*stmt); !eval_res.ok())
      return eval_res;

  return utils::OkStatus();
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
  if (lhs.index() != rhs.index()) {
    return evaluation::False;
  }
  if (utils::holds_alternative<evaluation::Nil>(lhs)) {
    return evaluation::Boolean::make_true(
        utils::get<evaluation::Nil>(lhs).get_line());
  }
  if (utils::holds_alternative<evaluation::Boolean>(lhs)) {
    return utils::get<evaluation::Boolean>(lhs) ==
           utils::get<evaluation::Boolean>(rhs);
  }
  if (utils::holds_alternative<evaluation::String>(lhs)) {
    return utils::get<evaluation::String>(lhs) ==
           utils::get<evaluation::String>(rhs);
  }
  if (utils::holds_alternative<evaluation::Number>(lhs)) {
    return utils::get<evaluation::Number>(lhs) ==
           utils::get<evaluation::Number>(rhs);
  }
  return {evaluation::Error{"unimplemented deep equal"sv, 0}};
}
auto interpreter::visit_impl(const statement::Variable &stmt) const
    -> utils::Status {
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
    -> utils::Status {
  defer { expr_res.clear(); };

  if (auto eval_res = evaluate(*stmt.value); !eval_res.ok())
    return eval_res;
  stmts_res.emplace_back(expr_res);
  return utils::OkStatus();
}
auto interpreter::visit_impl(const statement::If &stmt) const -> utils::Status {
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
    -> utils::Status {
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
auto interpreter::visit_impl(const statement::For &stmt) const -> utils::Status {
  defer { expr_res.clear(); };

  if (stmt.initializer){
    if (auto res = execute(*stmt.initializer); !res.ok())
      return res;
  }
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
auto interpreter::visit_impl(const statement::IllegalStmt &stmt) const
    -> utils::Status {
  return utils::InvalidArgument(stmt.message);
}
auto interpreter::visit_impl(const statement::Expression &stmt) const
    -> utils::Status {
  return evaluate(*stmt.expr);
}
auto interpreter::visit_impl(const statement::Block &stmt) const
    -> utils::Status {
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
    -> utils::Status {
  return stmt.accept(*this);
}
auto interpreter::get_result_impl() const -> eval_result_t { return expr_res; }

auto interpreter::evaluate_impl(const expression::Expr &expr) const
    -> utils::Status {
  return expr_res.set(expr.accept(*this))
      .visit(match{[](const evaluation::Error &e) {
                     return utils::InvalidArgument(e.to_string_view());
                   },
                   [](const utils::Monostate &) {
                     return utils::EmptyInput("no expr was evaluated.");
                   },
                   [](const auto &) { return utils::OkStatus(); }});
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
  dbg(critical, "unreachable code reached: {}", LOXOGRAPH_STACKTRACE);
  contract_assert(false);
  return {};
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
    if (utils::holds_alternative<evaluation::Boolean>(result)) {
      return evaluation::Boolean{!utils::get<evaluation::Boolean>(result)};
    }
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
  // TODO: here my logic went away. fixme here.
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

auto interpreter::visit_impl(const expression::IllegalExpr &expr) const
    -> eval_result_t {
  return evaluation::Error{"Illegal expression"s, expr.token.line};
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
  for (const auto &result : stmts_res)
    // TODO: temporary solution: skip newline if the result is empty(Monostate)
    if (auto str = value_to_string(format_policy, result); !str.empty())
      result_str += std::move(str) + "\n";

  return result_str;
}
LOXOGRAPH_API void delete_interpreter_fwd(interpreter *ptr) { delete ptr; }
} // namespace net::ancillarycat::loxograph
