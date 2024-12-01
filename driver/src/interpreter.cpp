#include <concepts>
#include <memory>
#include <typeinfo>

#include "config.hpp"
#include "fmt.hpp"
#include "loxo_fwd.hpp"

#include "expression.hpp"
#include "interpreter.hpp"

#include "Variant.hpp"

namespace net::ancillarycat::loxograph {
using utils::match;
utils::Status interpreter::interpret(
    const std::span<std::shared_ptr<statement::Stmt>> stmts) const {
  for (const auto &stmt : stmts) {
    if (auto eval_res = execute(*stmt); !eval_res.ok()) {
      this->expr_res.emplace<utils::Monostate>();
      return eval_res;
    }
  }
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
interpreter::eval_result_t
interpreter::is_deep_equal(const eval_result_t &lhs,
                           const eval_result_t &rhs) const {
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
utils::Status interpreter::visit_impl(const statement::Variable &stmt) const {
  if (stmt.has_initilizer()) {
    if (auto eval_res = evaluate(*stmt.initializer); !eval_res.ok())
      return eval_res;
    contract_assert(!!std::any_cast<string_view_type>(&stmt.name.literal),
                    1,
                    "variable name should be a string");
    dbg(info,
        "variable name: {}, value: {}",
        std::any_cast<string_view_type>(stmt.name.literal),
        expr_res.underlying_string());
    // string view fgailed again; not null-terminated
    env.add(stmt.name.to_string(utils::kTokenOnly), expr_res, stmt.name.line)
        .ignore_error();
    // TODO: reset expr_res or not???
    expr_res.emplace<utils::Monostate>();
  }
  return utils::OkStatus();
}
utils::Status interpreter::visit_impl(const statement::Print &stmt) const {
  if (auto eval_res = evaluate(*stmt.value); !eval_res.ok())
    return eval_res;
  stmts_res.emplace_back(expr_res);
  expr_res.emplace<utils::Monostate>();
  return utils::OkStatus();
}
utils::Status interpreter::visit_impl(const statement::IllegalStmt &stmt) const {
  return utils::InvalidArgument(stmt.message);
}
utils::Status interpreter::visit_impl(const statement::Expression &stmt) const {
  return evaluate(*stmt.expr);
}
utils::Status interpreter::execute_impl(const statement::Stmt &stmt) const {
  return stmt.accept(*this);
}
interpreter::eval_result_t interpreter::get_result_impl() const {
  return expr_res;
}

utils::Status interpreter::evaluate_impl(const expression::Expr &expr) const {
  expr_res = expr.accept(*this);
  return expr_res.visit(
      match{[](const evaluation::Error &e) {
              return utils::InvalidArgument(e.to_string_view());
            },
            [](const utils::Monostate &) {
              return utils::EmptyInput("no expr was evaluated.");
            },
            [](const auto &) { return utils::OkStatus(); }});
  /// @note ^^^^ you cannot just put the `expr.accept(*this)` here, it'll be
  ///               called more than once.
}
interpreter::eval_result_t
interpreter::visit_impl(const expression::Literal &expr) const {
  dbg(info, "literal type: {}", expr.literal.type);
  if (expr.literal.type.type == TokenType::kMonostate) {
    dbg(critical, "should not happen.");
    contract_assert(false);
    return {};
  }
  if (expr.literal.type.type == TokenType::kNil) {
    return evaluation::Nil{expr.literal.line};
  }
  if (expr.literal.type.type == TokenType::kTrue) {
    return evaluation::Boolean{true, expr.literal.line};
  }
  if (expr.literal.type.type == TokenType::kFalse) {
    return evaluation::Boolean{false, expr.literal.line};
  }
  if (expr.literal.type.type == TokenType::kString) {
    return evaluation::String{
        std::any_cast<string_view_type>(expr.literal.literal),
        expr.literal.line};
  }
  if (expr.literal.type.type == TokenType::kNumber) {
    return evaluation::Number{std::any_cast<long double>(expr.literal.literal),
                              expr.literal.line};
  }
  // temporary solution.
  return evaluation::Error{"Expected literal value"s, expr.literal.line};
}

interpreter::eval_result_t
interpreter::visit_impl(const expression::Unary &expr) const {
  auto inner_expr = expr.expr->accept(*this);
  if (expr.op.type == TokenType::kMinus) {
    if (utils::holds_alternative<evaluation::Number>(inner_expr)) {
      auto value = utils::get<evaluation::Number>(inner_expr);
      dbg(trace, "unary minus: {}", value);
      return evaluation::Number{value * (-1)};
    }
    // todo: error handling
    return evaluation::Error{"Operand must be a number."s, expr.op.line};
  }
  if (expr.op.type == TokenType::kBang) {
    auto value = is_true_value(inner_expr);
    dbg(trace, "unary bang: {}", value);
    return evaluation::Boolean{!value};
  }
  dbg(critical, "unreachable code reached: {}", LOXOGRAPH_STACKTRACE);
  contract_assert(false);
  // todo
  return {};
}

interpreter::eval_result_t
interpreter::visit_impl(const expression::Binary &expr) const {
  auto lhs = expr.left->accept(*this);
  auto rhs = expr.right->accept(*this);
  if (expr.op.type == TokenType::kEqualEqual) {
    return is_deep_equal(lhs, rhs);
  }
  if (expr.op.type == TokenType::kBangEqual) {
    auto result = is_deep_equal(lhs, rhs);
    if (utils::holds_alternative<evaluation::Boolean>(result)) {
      return evaluation::Boolean{!utils::get<evaluation::Boolean>(result)};
    }
    return result;
  }
  if (auto ptr = utils::get_if<evaluation::Error>(&lhs); ptr != nullptr) {
    return *ptr;
  }
  if (auto ptr = utils::get_if<evaluation::Error>(&rhs); ptr != nullptr) {
    return *ptr;
  }
  if (lhs.index() != rhs.index()) {
    dbg(error, "type mismatch: lhs: {}, rhs: {}", lhs.index(), rhs.index());
    dbg(warn, "current implementation only support same type binary operation");
    return evaluation::Error{"Operands must be two numbers or two strings."s,
                             expr.op.line};
  }
  if (utils::holds_alternative<evaluation::String>(lhs)) {
    if (expr.op.type == TokenType::kPlus) {
      return evaluation::String{utils::get<evaluation::String>(lhs) +
                                utils::get<evaluation::String>(rhs)};
    }
  }
  if (utils::holds_alternative<evaluation::Number>(lhs)) {
    auto real_lhs = utils::get<evaluation::Number>(lhs);
    auto real_rhs = utils::get<evaluation::Number>(rhs);
    switch (expr.op.type.type) {
    case TokenType::kMinus:
      return {evaluation::Number{real_lhs - real_rhs}};
    case TokenType::kPlus:
      return {evaluation::Number{real_lhs + real_rhs}};
    case TokenType::kSlash:
      return {evaluation::Number{real_lhs / real_rhs}};
    case TokenType::kStar:
      return {evaluation::Number{real_lhs * real_rhs}};
    case TokenType::kGreater:
      return {evaluation::Boolean{real_lhs > real_rhs}};
    case TokenType::kGreaterEqual:
      return {evaluation::Boolean{real_lhs >= real_rhs}};
    case TokenType::kLess:
      return {evaluation::Boolean{real_lhs < real_rhs}};
    case TokenType::kLessEqual:
      return {evaluation::Boolean{real_lhs <= real_rhs}};
    default:
      break;
    }
  }
  dbg(error, "unimplemented binary operator: {}", expr.op.to_string());
  contract_assert(false);
  return evaluation::Error{"unimplemented binary operator"s, expr.op.line};
}
interpreter::eval_result_t
interpreter::visit_impl(const expression::Grouping &expr) const {
  return {expr.expr->accept(*this)};
}
interpreter::eval_result_t
interpreter::visit_impl(const expression::Variable &expr) const {
  contract_assert(!!std::any_cast<string_view_type>(&expr.name.literal),
                  1,
                  "variable name should be a string");
  auto res = env.get(expr.name.to_string(utils::FormatPolicy::kTokenOnly));
  if (utils::holds_alternative<utils::Monostate>(res)) {
    // TODO temporary solution
    // add line info
    return {evaluation::Error{
        utils::format("Undefined variable '{}'.",
                      expr.name.to_string(utils::FormatPolicy::kTokenOnly)),
        expr.name.line}};
  }
  return res;
}
interpreter::eval_result_t
interpreter::visit_impl(const expression::IllegalExpr &expr) const {
  return evaluation::Error{"Illegal expression"s, expr.token.line};
}

auto interpreter::expr_to_string(const utils::FormatPolicy &format_policy) const
    -> string_type {
  return value_to_string(format_policy, expr_res);
}
auto interpreter::value_to_string(const utils::FormatPolicy &format_policy,
                                  const eval_result_t &value) const
    -> string_type {
  return value.underlying_string();
}
auto interpreter::to_string_impl(const utils::FormatPolicy &format_policy) const
    -> string_type {
  dbg(info, "expr_res index: {}", expr_res.index());
  dbg(info, "stmts size: {}", stmts_res.size());
  if (stmts_res.empty()) { // we are parse an expression, not a statement
    return value_to_string(format_policy, expr_res);
  }
  contract_assert(utils::holds_alternative<utils::Monostate>(expr_res),
                  1,
                  "expr_res should be empty");
  string_type result_str;
  for (const auto &result : stmts_res) {
    result_str += value_to_string(format_policy, result);
    result_str += "\n";
  }
  return result_str;
}
LOXOGRAPH_API void delete_interpreter_fwd(interpreter *ptr) { delete ptr; }
} // namespace net::ancillarycat::loxograph
