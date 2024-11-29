#include <string>

#include "config.hpp"
#include "loxo_fwd.hpp"
#include "fmt.hpp"

#include "expression.hpp"
#include "ExprVisitor.hpp"
#include "Evaluatable.hpp"

namespace net::ancillarycat::loxograph::expression {
Expr::expr_result_t Literal::accept_impl(const ExprVisitor &visitor) const {
  return visitor.visit(*this);
}
Expr::string_type Literal::to_string_impl(const utils::FormatPolicy &) const {
  return literal.to_string(utils::FormatPolicy::kTokenOnly);
}
Expr::expr_result_t Unary::accept_impl(const ExprVisitor &visitor) const {
  return visitor.visit(*this);
}
Expr::string_type
Unary::to_string_impl(const utils::FormatPolicy &format_policy) const {
  return "(" + op.to_string(utils::FormatPolicy::kTokenOnly) + " " +
         expr->to_string() + ")";
}
Expr::expr_result_t Binary::accept_impl(const ExprVisitor &visitor) const {
  return visitor.visit(*this);
}
Expr::string_type
Binary::to_string_impl(const utils::FormatPolicy &format_policy) const {
  return "(" + op.to_string(utils::FormatPolicy::kTokenOnly) + " " +
         left->to_string() + " " + right->to_string() + ")";
}
Expr::expr_result_t Variable::accept_impl(const ExprVisitor &visitor) const {
  return {std::monostate{}};
}
auto Variable::to_string_impl(const utils::FormatPolicy &format_policy) const
    -> string_type {
  return {};
}
Expr::expr_result_t Grouping::accept_impl(const ExprVisitor &visitor) const {
  return visitor.visit(*this);
}
Expr::string_type
Grouping::to_string_impl(const utils::FormatPolicy &format_policy) const {
  /// strange print format, but codecrafter's test needs this.
  return "(group " + expr->to_string() + ")";
}
Expr::expr_result_t IllegalExpr::accept_impl(const ExprVisitor &visitor) const {
  return visitor.visit(*this);
}
std::string
IllegalExpr::to_string_impl(const utils::FormatPolicy &format_policy) const {
  return utils::format("[line {}] Error at '{}': {}",
                       token.line,
                       token.to_string(utils::FormatPolicy::kTokenOnly),
                       error.message());
}
} // namespace net::ancillarycat::loxograph::expression
