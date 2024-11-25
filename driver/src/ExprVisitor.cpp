#include <any>
#include <ostream>
#include <source_location>
#include <sstream>

#include "config.hpp"
#include "loxo_fwd.hpp"

#include "Expr.hpp"
#include "ExprVisitor.hpp"
#include "status.hpp"
#include "SyntaxLiteral.hpp"

namespace net::ancillarycat::loxograph::expression {
ExprVisitor::expr_result_t
ExprEvaluator::is_true_value(const expr_result_t &value) const {
  if (!value.has_value() or value.type() == typeid(syntax::Nil)) {
    return syntax::False;
  }

  if (value.type() == typeid(syntax::Boolean)) {
    return value;
  }
  // fixme: double 0 is false or not?
  // if (value.type() == typeid(long double)){
  //   return {*utils::cast_literal<long double>(value) != 0L};
  // }
  return syntax::True;
}
bool ExprEvaluator::is_deep_equal(const expr_result_t &lhs,
                                  const expr_result_t &rhs) const {
  /// TODO: how on earth can i implement this?
  return false; // TODO: temporary solution
}
utils::Status ExprEvaluator::evaluate(const Expr &expr) const try {
  const_cast<expr_result_t &>(res) = expr.accept(*this);
  return utils::OkStatus();
} catch (const std::exception &e) {
  return {utils::InvalidArgument(e.what())};
}
ExprVisitor::expr_result_t
ExprEvaluator::visit_impl(const Literal &expr) const {
  // FIXME: a bug. token has type recorded, and some token such as `true`,
  //        `false`, `nil` has their type, but no literal i put into this any.
  //        now temporary solution: in lex add some of them into literal.
  //        shall be fixed in the future.
  // return expr.literal.literal; // <- std::any
  // Expr ^^^ Token ^^^
  dbg(info, "literal type: {}", expr.literal.type);
  if (expr.literal.type.type == TokenType::kMonostate) {
    dbg(critical, "should not happen.");
    contract_assert(false);
    return {};
  }
  if (expr.literal.type.type == TokenType::kNil) {
    return {syntax::Nil{}};
  }
  if (expr.literal.type.type == TokenType::kTrue) {
    return {syntax::True};
  }
  if (expr.literal.type.type == TokenType::kFalse) {
    return {syntax::False};
  }
  if (expr.literal.type.type == TokenType::kString) {
    return {
        syntax::String{*utils::get_if<string_view_type>(expr.literal.literal)}};
  }
  // temporary solution.
  return expr.literal.literal;
}
ExprVisitor::expr_result_t ExprEvaluator::visit_impl(const Unary &expr) const {
  auto inner_expr =
      expr.expr->accept(*this); // fixme: bug here, infinite recursion
  if (expr.op.type == TokenType::kMinus) {
    if (auto ptr = utils::get_if<long double>(inner_expr)) {
      return {-(*ptr)};
    }
    return {};
  }
  if (expr.op.type == TokenType::kBang) {
    auto value = is_true_value(inner_expr);
    if (auto ptr = utils::get_if<syntax::Boolean>(value)) {
      dbg(trace, "unary not: {}", *ptr);
      return {syntax::Boolean{!(*ptr)}};
    }
    dbg(error, "unreachable code reached: {}", LOXOGRAPH_STACKTRACE);
  }
  dbg(critical, "unreachable code reached: {}", LOXOGRAPH_STACKTRACE);
  contract_assert(false);
  return {};
}
ExprVisitor::expr_result_t ExprEvaluator::visit_impl(const Binary &expr) const {
  auto lhs = expr.left->accept(*this); // shall get a string or a number
  auto rhs = expr.right->accept(*this);
  if (lhs.type() != rhs.type()) {
    dbg(error,
        "type mismatch: lhs: {}, rhs: {}",
        lhs.type().name(),
        rhs.type().name());
    dbg(warn, "current implementation only support same type binary operation");
    return {};
  }
  if (expr.op.type == TokenType::kBangEqual or
      expr.op.type == TokenType::kEqualEqual) {
    return {is_deep_equal(lhs, rhs)};
  }
  if (lhs.type() == typeid(syntax::String)) {
    if (expr.op.type == TokenType::kPlus) {
      return {syntax::String{std::any_cast<syntax::String>(lhs) +
                             std::any_cast<syntax::String>(rhs)}};
    }
  }
  if (lhs.type() == typeid(long double)) {
    auto real_lhs = utils::get_if<long double>(lhs);
    auto real_rhs = utils::get_if<long double>(rhs);
    contract_assert(real_lhs and real_rhs);
    switch (expr.op.type.type) {
    case TokenType::kMinus:
      return {*real_lhs - *real_rhs};
    case TokenType::kPlus:
      return {*real_lhs + *real_rhs};
    case TokenType::kSlash:
      return {*real_lhs / *real_rhs};
    case TokenType::kStar:
      return {*real_lhs * *real_rhs};
    case TokenType::kGreater:
      return syntax::Boolean{*real_lhs > *real_rhs};
    case TokenType::kGreaterEqual:
      return syntax::Boolean{*real_lhs >= *real_rhs};
    case TokenType::kLess:
      return syntax::Boolean{*real_lhs < *real_rhs};
    case TokenType::kLessEqual:
      return syntax::Boolean{*real_lhs <= *real_rhs};
    default:
      break;
    }
  }
  dbg(error, "unimplemented binary operator: {}", expr.op.to_string());
  contract_assert(false);
  return {};
}
ExprVisitor::expr_result_t
ExprEvaluator::visit_impl(const Grouping &expr) const {
  return {expr.expr->accept(*this)};
}
ExprVisitor::expr_result_t
ExprEvaluator::visit_impl(const IllegalExpr &expr) const {
  return {};
}
auto ExprEvaluator::to_string_impl(
    const utils::FormatPolicy &format_policy) const -> string_type {
  if (!res.has_value())
    return "<no result>";
  if (res.type() == typeid(long double))
  // return utils::format("{:.1f}", *utils::get_if<long double>(res));
  {
    auto ptr = utils::get_if<long double>(res);
    if (utils::is_integer(*ptr)) {
      // print as integer(no decimal point)
      return utils::format("{:.0f}", *ptr);
    }
// print as-is
#if AC_CPP_DEBUG
    return utils::format(
        "{}",
        *ptr); // std::format failed to handle `.f` without a number;
#else
    return utils::format("{}", *ptr);
#endif
  }
  if (res.type() == typeid(syntax::String))
    return utils::get_if<syntax::String>(res)->to_string();
  if (res.type() == typeid(syntax::Boolean))
    return utils::get_if<syntax::Boolean>(res)->to_string();
  if (res.type() == typeid(syntax::Nil))
    return "nil"s;
  dbg(error, "unimplemented type: {}", res.type().name());
  return "<unknown type>";
}

ExprVisitor::expr_result_t ASTPrinter::visit_impl(const Literal &expr) const {
  dbg(info, "Literal: {}", expr.to_string());
  oss << expr << std::endl;
  return {};
}
ExprVisitor::expr_result_t ASTPrinter::visit_impl(const Unary &expr) const {
  dbg(info, "Unary: {}", expr.to_string());
  oss << expr << std::endl;
  return {};
}
ExprVisitor::expr_result_t ASTPrinter::visit_impl(const Binary &expr) const {
  dbg(info, "Binary: {}", expr.to_string());
  oss << expr << std::endl;
  return {};
}
ExprVisitor::expr_result_t ASTPrinter::visit_impl(const Grouping &expr) const {
  dbg(info, "Grouping: {}", expr.to_string());
  oss << expr << std::endl;
  return {};
}
ExprVisitor::expr_result_t
ASTPrinter::visit_impl(const IllegalExpr &expr) const {
  dbg(info, "IllegalExpr: {}", expr.to_string());
  error_stream << expr << std::endl;
  return {};
}
utils::Printable::string_type
ASTPrinter::to_string_impl(const utils::FormatPolicy &) const {
  return oss.str();
}
} // namespace net::ancillarycat::loxograph::expression
