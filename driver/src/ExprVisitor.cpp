#include <any>
#include <ostream>
#include <source_location>
#include <sstream>
#include <utility>
#include <variant>

#include "config.hpp"
#include "loxo_fwd.hpp"

#include "Evaluatable.hpp"
#include "Expr.hpp"
#include "ExprVisitor.hpp"
#include "fmt.hpp"
#include "status.hpp"

namespace net::ancillarycat::loxograph::expression {
eval::Boolean ExprEvaluator::is_true_value(const value_t &value) const {
  if (std::holds_alternative<eval::Nil>(value)) {
    return eval::Boolean::make_false(std::get<eval::Nil>(value).get_line());
  }

  if (std::holds_alternative<eval::Boolean>(value)) {
    return std::get<eval::Boolean>(value);
  }

  // fixme: double 0 is false or not?
  return eval::True;
}
ExprEvaluator::value_t ExprEvaluator::is_deep_equal(const value_t &lhs,
                                                    const value_t &rhs) const {
  if (lhs.index() != rhs.index()) {
    return eval::False;
  }
  if (std::holds_alternative<eval::Nil>(lhs)) {
    return eval::Boolean::make_true(std::get<eval::Nil>(lhs).get_line());
  }
  if (std::holds_alternative<eval::Boolean>(lhs)) {
    return std::get<eval::Boolean>(lhs) == std::get<eval::Boolean>(rhs);
  }
  if (std::holds_alternative<eval::String>(lhs)) {
    return std::get<eval::String>(lhs) == std::get<eval::String>(rhs);
  }
  if (std::holds_alternative<eval::Number>(lhs)) {
    return std::get<eval::Number>(lhs) == std::get<eval::Number>(rhs);
  }
  return eval::ErrorSyntax{"unimplemented deep equal"sv, 0};
}
ExprVisitor::value_t ExprEvaluator::get_result_impl() const { return res; }

utils::Status ExprEvaluator::evaluate_impl(const Expr &expr) const {
  const_cast<value_t &>(res) = expr.accept(*this);
  if (std::holds_alternative<eval::ErrorSyntax>(res)) {
    return utils::InvalidArgument(
        std::get<eval::ErrorSyntax>(res).to_string());
  }
  if (std::holds_alternative<std::monostate>(res)) {
    return utils::EmptyInput("no expr was evaluated.");
  }
  return utils::OkStatus();
}
ExprEvaluator::value_t ExprEvaluator::visit_impl(const Literal &expr) const {
  dbg(info, "literal type: {}", expr.literal.type);
  if (expr.literal.type.type == TokenType::kMonostate) {
    dbg(critical, "should not happen.");
    contract_assert(false);
    return {};
  }
  if (expr.literal.type.type == TokenType::kNil) {
    return eval::Nil{expr.literal.line};
  }
  if (expr.literal.type.type == TokenType::kTrue) {
    return eval::Boolean{true, expr.literal.line};
  }
  if (expr.literal.type.type == TokenType::kFalse) {
    return eval::Boolean{false, expr.literal.line};
  }
  if (expr.literal.type.type == TokenType::kString) {
    return eval::String{std::any_cast<string_view_type>(expr.literal.literal),
                          expr.literal.line};
  }
  if (expr.literal.type.type == TokenType::kNumber) {
    return eval::Number{std::any_cast<long double>(expr.literal.literal),
                          expr.literal.line};
  }
  // temporary solution.
  // return expr.literal.literal;
  return eval::ErrorSyntax{"Expected literal value"s, expr.literal.line};
}

ExprEvaluator::value_t ExprEvaluator::visit_impl(const Unary &expr) const {
  auto inner_expr = expr.expr->accept(*this);
  if (expr.op.type == TokenType::kMinus) {
    if (std::holds_alternative<eval::Number>(inner_expr)) {
      auto value = std::get<eval::Number>(inner_expr);
      dbg(trace, "unary minus: {}", value);
      return eval::Number{value * (-1)};
    }
    // todo: error handling
    return eval::ErrorSyntax{"Operand must be a number."s, expr.op.line};
  }
  if (expr.op.type == TokenType::kBang) {
    auto value = is_true_value(inner_expr);
    dbg(trace, "unary bang: {}", value);
    return eval::Boolean{!value};
  }
  dbg(critical, "unreachable code reached: {}", LOXOGRAPH_STACKTRACE);
  contract_assert(false);
  // todo
  return {};
}

ExprEvaluator::value_t ExprEvaluator::visit_impl(const Binary &expr) const {
  auto lhs = expr.left->accept(*this);
  auto rhs = expr.right->accept(*this);
  if (expr.op.type == TokenType::kEqualEqual) {
    return is_deep_equal(lhs, rhs);
  }
  if (expr.op.type == TokenType::kBangEqual) {
    auto result = is_deep_equal(lhs, rhs);
    if (std::holds_alternative<eval::Boolean>(result)) {
      return eval::Boolean{!std::get<eval::Boolean>(result)};
    }
    return result;
  }
  if (lhs.index() != rhs.index()) {
    dbg(error, "type mismatch: lhs: {}, rhs: {}", lhs.index(), rhs.index());
    dbg(warn, "current implementation only support same type binary operation");
    return eval::ErrorSyntax{"Operands must be two numbers or two strings."s,
                               expr.op.line};
  }
  if (std::holds_alternative<eval::String>(lhs)) {
    if (expr.op.type == TokenType::kPlus) {
      return eval::String{std::get<eval::String>(lhs) +
                            std::get<eval::String>(rhs)};
    }
  }
  if (std::holds_alternative<eval::Number>(lhs)) {
    auto real_lhs = std::get<eval::Number>(lhs);
    auto real_rhs = std::get<eval::Number>(rhs);
    switch (expr.op.type.type) {
    case TokenType::kMinus:
      return eval::Number{real_lhs - real_rhs};
    case TokenType::kPlus:
      return eval::Number{real_lhs + real_rhs};
    case TokenType::kSlash:
      return eval::Number{real_lhs / real_rhs};
    case TokenType::kStar:
      return eval::Number{real_lhs * real_rhs};
    case TokenType::kGreater:
      return eval::Boolean{real_lhs > real_rhs};
    case TokenType::kGreaterEqual:
      return eval::Boolean{real_lhs >= real_rhs};
    case TokenType::kLess:
      return eval::Boolean{real_lhs < real_rhs};
    case TokenType::kLessEqual:
      return eval::Boolean{real_lhs <= real_rhs};
    default:
      break;
    }
  }
  dbg(error, "unimplemented binary operator: {}", expr.op.to_string());
  contract_assert(false);
  return eval::ErrorSyntax{"unimplemented binary operator"s, expr.op.line};
}
ExprEvaluator::value_t ExprEvaluator::visit_impl(const Grouping &expr) const {
  return {expr.expr->accept(*this)};
}
ExprEvaluator::value_t
ExprEvaluator::visit_impl(const IllegalExpr &expr) const {
  return eval::ErrorSyntax{"Illegal expression"s, expr.token.line};
}

auto ASTPrinter::evaluate_impl(const Expr &expr) const -> utils::Status {
  const_cast<value_t&>(res) =  expr.accept(*this);
  return utils::Status::kOkStatus;
}
ExprEvaluator::value_t ASTPrinter::visit_impl(const Literal &expr) const {
  dbg(info, "Literal: {}", expr.to_string());
  oss << expr << std::endl;
  return {};
}
ExprEvaluator::value_t ASTPrinter::visit_impl(const Unary &expr) const {
  dbg(info, "Unary: {}", expr.to_string());
  oss << expr << std::endl;
  return {};
}
ExprEvaluator::value_t ASTPrinter::visit_impl(const Binary &expr) const {
  dbg(info, "Binary: {}", expr.to_string());
  oss << expr << std::endl;
  return {};
}

auto ExprEvaluator::to_string_impl(
    const utils::FormatPolicy &format_policy) const -> string_type {
  if (std::holds_alternative<std::monostate>(res))
    return "<no result>";
  if (std::holds_alternative<eval::Number>(res))
    return std::get<eval::Number>(res).to_string();
  if (std::holds_alternative<eval::String>(res))
    return std::get<eval::String>(res).to_string();
  if (std::holds_alternative<eval::Boolean>(res))
    return std::get<eval::Boolean>(res).to_string();
  if (std::holds_alternative<eval::Nil>(res))
    return "nil"s;
  if (std::holds_alternative<eval::ErrorSyntax>(res))
    return std::get<eval::ErrorSyntax>(res).to_string();
  dbg(error, "unimplemented type: {}", res.index());
  return "<unknown type>";
}

ExprEvaluator::value_t ASTPrinter::visit_impl(const Grouping &expr) const {
  dbg(info, "Grouping: {}", expr.to_string());
  oss << expr << std::endl;
  return {};
}
ExprEvaluator::value_t ASTPrinter::visit_impl(const IllegalExpr &expr) const {
  dbg(info, "IllegalExpr: {}", expr.to_string());
  error_stream << expr << std::endl;
  return {};
}
auto ASTPrinter::to_string_impl(const utils::FormatPolicy &format_policy) const
    -> string_type {
  return oss.str();
}
auto ASTPrinter::to_string_view_impl(const utils::FormatPolicy &) const
    -> utils::Viewable::string_view_type {
  return oss.view();
}
ExprVisitor::value_t ASTPrinter::get_result_impl() const { return res; }
} // namespace net::ancillarycat::loxograph::expression
