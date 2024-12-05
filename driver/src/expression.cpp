#include <string>

#include "config.hpp"
#include "loxo_fwd.hpp"
#include "utils.hpp"

#include "expression.hpp"
#include "ExprVisitor.hpp"
#include "Evaluatable.hpp"

namespace net::ancillarycat::loxo::expression {
Literal::Literal(token_t &&literal) : literal(std::move(literal)) {}
Expr::expr_result_t Literal::accept_impl(const ExprVisitor &visitor) const {
  return visitor.visit(*this);
}
Expr::string_type Literal::to_string_impl(const utils::FormatPolicy &) const {
  return literal.to_string(utils::FormatPolicy::kTokenOnly);
}
Unary::Unary(token_t &&op, expr_ptr_t &&expr)
    : op(std::move(op)), expr(std::move(expr)) {}
Expr::expr_result_t Unary::accept_impl(const ExprVisitor &visitor) const {
  return visitor.visit(*this);
}
Expr::string_type
Unary::to_string_impl(const utils::FormatPolicy &format_policy) const {
  return "(" + op.to_string(utils::FormatPolicy::kTokenOnly) + " " +
         expr->to_string() + ")";
}
Binary::Binary(token_t &&op, expr_ptr_t &&left, expr_ptr_t &&right)
    : op(std::move(op)), left(std::move(left)), right(std::move(right)) {}
Expr::expr_result_t Binary::accept_impl(const ExprVisitor &visitor) const {
  return visitor.visit(*this);
}
Expr::string_type
Binary::to_string_impl(const utils::FormatPolicy &format_policy) const {
  return "(" + op.to_string(utils::FormatPolicy::kTokenOnly) + " " +
         left->to_string() + " " + right->to_string() + ")";
}
Variable::Variable(token_t &&name) : name(std::move(name)) {}
Expr::expr_result_t Variable::accept_impl(const ExprVisitor &visitor) const {
  return visitor.visit(*this);
}
auto Variable::to_string_impl(const utils::FormatPolicy &format_policy) const
    -> string_type {
  return {};
}
Grouping::Grouping(expr_ptr_t &&expr) : expr(std::move(expr)) {}
Expr::expr_result_t Grouping::accept_impl(const ExprVisitor &visitor) const {
  return visitor.visit(*this);
}
Expr::string_type
Grouping::to_string_impl(const utils::FormatPolicy &format_policy) const {
  /// strange print format, but codecrafter's test needs this.
  return "(group " + expr->to_string() + ")";
}
Expr::expr_result_t Assignment::accept_impl(const ExprVisitor &visitor) const {
  return visitor.visit(*this);
}
Assignment::Assignment(token_t &&name, expr_ptr_t &&value)
    : name(std::move(name)), value_expr(std::move(value)) {}
auto Assignment::to_string_impl(const utils::FormatPolicy &format_policy) const
    -> string_type {
  TODO();
}
Logical::Logical(token_t &&op, expr_ptr_t &&left, expr_ptr_t &&right)
    : op(std::move(op)), left(std::move(left)), right(std::move(right)) {}
Expr::expr_result_t Logical::accept_impl(const ExprVisitor &visitor) const {
  return visitor.visit(*this);
}
Call::Call(expr_ptr_t &&callee,
           token_t &&paren,
           std::vector<expr_ptr_t> &&arguments)
    : callee(std::move(callee)), paren(std::move(paren)),
      args(std::move(arguments)) {}
auto Logical::to_string_impl(const utils::FormatPolicy &format_policy) const
    -> string_type {
  return "(" + left->to_string() + " " + op.to_string() + " " +
         right->to_string() + ")";
}
Expr::expr_result_t Call::accept_impl(const ExprVisitor &visitor) const {
  return visitor.visit(*this);
}
auto Call::to_string_impl(const utils::FormatPolicy &format_policy) const
    -> string_type {
  TODO(...)
}
IllegalExpr::IllegalExpr(token_t token, parse_error error)
    : token(std::move(token)), error(std::move(error)) {}
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
} // namespace net::ancillarycat::loxo::expression
