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
#include "status.hpp"

namespace net::ancillarycat::loxograph::expression {
syntax::Boolean ExprEvaluator::is_true_value(const value_t &value) const {
  if (std::holds_alternative<syntax::Nil>(value)) {
    return syntax::Boolean::make_false(std::get<syntax::Nil>(value).get_line());
  }

  if (std::holds_alternative<syntax::Boolean>(value)) {
    return std::get<syntax::Boolean>(value);
  }

  // fixme: double 0 is false or not?
  return syntax::True;
}
ExprEvaluator::value_t ExprEvaluator::is_deep_equal(const value_t &lhs,
                                                    const value_t &rhs) const {
  if (lhs.index() != rhs.index()) {
    return syntax::False;
  }
  if (std::holds_alternative<syntax::Nil>(lhs)) {
    return syntax::Boolean::make_true(std::get<syntax::Nil>(lhs).get_line());
  }
  if (std::holds_alternative<syntax::Boolean>(lhs)) {
    return std::get<syntax::Boolean>(lhs) == std::get<syntax::Boolean>(rhs);
  }
  if (std::holds_alternative<syntax::String>(lhs)) {
    return std::get<syntax::String>(lhs) == std::get<syntax::String>(rhs);
  }
  if (std::holds_alternative<syntax::Number>(lhs)) {
    return std::get<syntax::Number>(lhs) == std::get<syntax::Number>(rhs);
  }
  return syntax::ErrorSyntax{"unimplemented deep equal"sv, 0};
}
utils::Status ExprEvaluator::evaluate(const Expr &expr) const {
  const_cast<value_t &>(res) = expr.accept(*this);
  if (std::holds_alternative<syntax::ErrorSyntax>(res)) {
    return utils::InvalidArgument(std::get<syntax::ErrorSyntax>(res).to_string());
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
    return syntax::Nil{expr.literal.line};
  }
  if (expr.literal.type.type == TokenType::kTrue) {
    return syntax::Boolean{true, expr.literal.line};
  }
  if (expr.literal.type.type == TokenType::kFalse) {
    return syntax::Boolean{false, expr.literal.line};
  }
  if (expr.literal.type.type == TokenType::kString) {
    return syntax::String{std::any_cast<string_view_type>(expr.literal.literal),
                          expr.literal.line};
  }
  if (expr.literal.type.type == TokenType::kNumber) {
    return syntax::Number{std::any_cast<long double>(expr.literal.literal),
                          expr.literal.line};
  }
  // temporary solution.
  // return expr.literal.literal;
  return syntax::ErrorSyntax{"Expected literal value"s, expr.literal.line};
}

ExprEvaluator::value_t ExprEvaluator::visit_impl(const Unary &expr) const {
  auto inner_expr = expr.expr->accept(*this);
  if (expr.op.type == TokenType::kMinus) {
    if (std::holds_alternative<syntax::Number>(inner_expr)) {
      auto value = std::get<syntax::Number>(inner_expr);
      dbg(trace, "unary minus: {}", value);
      return syntax::Number{value * (-1)};
    }
    // todo: error handling
    return syntax::ErrorSyntax{"Operand must be a number."s, expr.op.line};
  }
  if (expr.op.type == TokenType::kBang) {
    auto value = is_true_value(inner_expr);
    dbg(trace, "unary bang: {}", value);
    return syntax::Boolean{!value};
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
    if (std::holds_alternative<syntax::Boolean>(result)) {
      return syntax::Boolean{!std::get<syntax::Boolean>(result)};
    }
    return result;
  }
  if (lhs.index() != rhs.index()) {
    dbg(error, "type mismatch: lhs: {}, rhs: {}", lhs.index(), rhs.index());
    dbg(warn, "current implementation only support same type binary operation");
    return syntax::ErrorSyntax{"type mismatch"s, expr.op.line};
  }
  if (std::holds_alternative<syntax::String>(lhs)) {
    if (expr.op.type == TokenType::kPlus) {
      return syntax::String{std::get<syntax::String>(lhs) +
                            std::get<syntax::String>(rhs)};
    }
  }
  if (std::holds_alternative<syntax::Number>(lhs)) {
    auto real_lhs = std::get<syntax::Number>(lhs);
    auto real_rhs = std::get<syntax::Number>(rhs);
    switch (expr.op.type.type) {
    case TokenType::kMinus:
      return syntax::Number{real_lhs - real_rhs};
    case TokenType::kPlus:
      return syntax::Number{real_lhs + real_rhs};
    case TokenType::kSlash:
      return syntax::Number{real_lhs / real_rhs};
    case TokenType::kStar:
      return syntax::Number{real_lhs * real_rhs};
    case TokenType::kGreater:
      return syntax::Boolean{real_lhs > real_rhs};
    case TokenType::kGreaterEqual:
      return syntax::Boolean{real_lhs >= real_rhs};
    case TokenType::kLess:
      return syntax::Boolean{real_lhs < real_rhs};
    case TokenType::kLessEqual:
      return syntax::Boolean{real_lhs <= real_rhs};
    default:
      break;
    }
  }
  dbg(error, "unimplemented binary operator: {}", expr.op.to_string());
  contract_assert(false);
  return syntax::ErrorSyntax{"unimplemented binary operator"s, expr.op.line};
}
ExprEvaluator::value_t ExprEvaluator::visit_impl(const Grouping &expr) const {
  return {expr.expr->accept(*this)};
}
ExprEvaluator::value_t
ExprEvaluator::visit_impl(const IllegalExpr &expr) const {
  return syntax::ErrorSyntax{"Illegal expression"s, expr.token.line};
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
  if (std::holds_alternative<syntax::Number>(res))
    return std::get<syntax::Number>(res).to_string();
  if (std::holds_alternative<syntax::String>(res))
    return std::get<syntax::String>(res).to_string();
  if (std::holds_alternative<syntax::Boolean>(res))
    return std::get<syntax::Boolean>(res).to_string();
  if (std::holds_alternative<syntax::Nil>(res))
    return "nil"s;
  if (std::holds_alternative<syntax::ErrorSyntax>(res))
    return std::get<syntax::ErrorSyntax>(res).to_string();
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
} // namespace net::ancillarycat::loxograph::expression
