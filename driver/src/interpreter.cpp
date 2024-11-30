#include <concepts>
#include <memory>
#include <typeinfo>

#include "config.hpp"
#include "fmt.hpp"
#include "loxo_fwd.hpp"

#include "expression.hpp"
#include "interpreter.hpp"

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
  // if (std::holds_alternative<evaluation::Nil>(value)) {
  //   return evaluation::Boolean::make_false(
  //       std::get<evaluation::Nil>(value).get_line());
  // }
  // // fixme: double 0 is false or not?
  // return evaluation::True;
  return std::visit(match{
                        [](const evaluation::Nil &n) {
                          return evaluation::Boolean::make_false(n.get_line());
                        },
                        [](const evaluation::Boolean &b) { return b; },
                        [](const auto &) { return evaluation::True; },
                    },
                    value);
}
interpreter::eval_result_t
interpreter::is_deep_equal(const eval_result_t &lhs,
                           const eval_result_t &rhs) const {
  if (lhs.index() != rhs.index()) {
    return evaluation::False;
  }
  if (std::holds_alternative<evaluation::Nil>(lhs)) {
    return evaluation::Boolean::make_true(
        std::get<evaluation::Nil>(lhs).get_line());
  }
  if (std::holds_alternative<evaluation::Boolean>(lhs)) {
    return std::get<evaluation::Boolean>(lhs) ==
           std::get<evaluation::Boolean>(rhs);
  }
  if (std::holds_alternative<evaluation::String>(lhs)) {
    return std::get<evaluation::String>(lhs) ==
           std::get<evaluation::String>(rhs);
  }
  if (std::holds_alternative<evaluation::Number>(lhs)) {
    return std::get<evaluation::Number>(lhs) ==
           std::get<evaluation::Number>(rhs);
  }
  return {evaluation::Error{"unimplemented deep equal"sv, 0}};
}
utils::Status interpreter::visit_impl(const statement::Variable &) const {
  return {};
}
utils::Status interpreter::visit_impl(const statement::Print &stmt) const {
  if (auto eval_res = evaluate(*stmt.value); !eval_res.ok())
    return eval_res;
  stmts_res.emplace_back(expr_res);
  expr_res.emplace<utils::Monostate>();
  return utils::OkStatus();
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
  return std::visit(match{[](const evaluation::Error &e) {
                            return utils::InvalidArgument(e.to_string_view());
                          },
                          [](const utils::Monostate &) {
                            return utils::EmptyInput("no expr was evaluated.");
                          },
                          [](const auto &) { return utils::OkStatus(); }},
                    expr_res);
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
  // return expr.literal.literal;
  return evaluation::Error{"Expected literal value"s, expr.literal.line};
}

interpreter::eval_result_t
interpreter::visit_impl(const expression::Unary &expr) const {
  auto inner_expr = expr.expr->accept(*this);
  if (expr.op.type == TokenType::kMinus) {
    if (std::holds_alternative<evaluation::Number>(inner_expr)) {
      auto value = std::get<evaluation::Number>(inner_expr);
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
    if (std::holds_alternative<evaluation::Boolean>(result)) {
      return evaluation::Boolean{!std::get<evaluation::Boolean>(result)};
    }
    return result;
  }
  if (lhs.index() != rhs.index()) {
    dbg(error, "type mismatch: lhs: {}, rhs: {}", lhs.index(), rhs.index());
    dbg(warn, "current implementation only support same type binary operation");
    return evaluation::Error{"Operands must be two numbers or two strings."s,
                             expr.op.line};
  }
  if (std::holds_alternative<evaluation::String>(lhs)) {
    if (expr.op.type == TokenType::kPlus) {
      return evaluation::String{std::get<evaluation::String>(lhs) +
                                std::get<evaluation::String>(rhs)};
    }
  }
  if (std::holds_alternative<evaluation::Number>(lhs)) {
    auto real_lhs = std::get<evaluation::Number>(lhs);
    auto real_rhs = std::get<evaluation::Number>(rhs);
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
  // ReSharper disable once CppUseFamiliarTemplateSyntaxForGenericLambdas
  return std::visit(
      [=](auto &&val) -> string_type {
        if constexpr (std::is_same_v<std::decay_t<decltype(val)>,
                                     utils::Monostate>) {
          dbg(warn, "no result");
          return "<no result>"s;
        } else
          return val.to_string(format_policy);
      },
      value);
}
auto interpreter::to_string_impl(const utils::FormatPolicy &format_policy) const
    -> string_type {
  dbg(info, "expr_res index: {}", expr_res.index());
  dbg(info, "stmts size: {}", stmts_res.size());
  if (stmts_res.empty()) { // we are parse an expression, not a statement
    return value_to_string(format_policy, expr_res);
  }
  contract_assert(std::holds_alternative<utils::Monostate>(expr_res),
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
