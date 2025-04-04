#include <string>

#include "accat/auxilia/details/format.hpp"
#include "details/loxo_fwd.hpp"

#include "expression.hpp"
#include "ExprVisitor.hpp"
#include "Evaluatable.hpp"

namespace accat::loxo::expression {
using auxilia::FormatPolicy;
using enum auxilia::FormatPolicy;

Literal::Literal(token_t &&literal) : literal(std::move(literal)) {}
Expr::expr_result_t Literal::accept_impl(const ExprVisitor &visitor) const {
  return visitor.visit(*this);
}
Expr::string_type Literal::to_string(const FormatPolicy &) const {
  return literal.to_string(kDetailed);
}
Unary::Unary(token_t &&op, expr_ptr_t &&expr)
    : op(std::move(op)), expr(std::move(expr)) {}
Expr::expr_result_t Unary::accept_impl(const ExprVisitor &visitor) const {
  return visitor.visit(*this);
}
Expr::string_type Unary::to_string(const FormatPolicy &format_policy) const {
  return "(" + op.to_string(kDetailed) + " " + expr->to_string(format_policy) +
         ")";
}
Binary::Binary(token_t &&op, expr_ptr_t &&left, expr_ptr_t &&right)
    : op(std::move(op)), left(std::move(left)), right(std::move(right)) {}
Expr::expr_result_t Binary::accept_impl(const ExprVisitor &visitor) const {
  return visitor.visit(*this);
}
Expr::string_type Binary::to_string(const FormatPolicy &format_policy) const {
  return "(" + op.to_string(kDetailed) + " " + left->to_string(format_policy) +
         " " + right->to_string(format_policy) + ")";
}
Variable::Variable(token_t &&name) : name(std::move(name)) {}
Expr::expr_result_t Variable::accept_impl(const ExprVisitor &visitor) const {
  return visitor.visit(*this);
}
auto Variable::to_string(const FormatPolicy &format_policy) const
    -> string_type {
  return name.to_string(kDetailed);
}
Grouping::Grouping(expr_ptr_t &&expr) : expr(std::move(expr)) {}
Expr::expr_result_t Grouping::accept_impl(const ExprVisitor &visitor) const {
  return visitor.visit(*this);
}
Expr::string_type Grouping::to_string(const FormatPolicy &format_policy) const {
  /// strange print format, but codecrafter's test needs this.
  return "(group " + expr->to_string(format_policy) + ")";
}
Expr::expr_result_t Assignment::accept_impl(const ExprVisitor &visitor) const {
  return visitor.visit(*this);
}
Assignment::Assignment(token_t &&name, expr_ptr_t &&value)
    : name(std::move(name)), value_expr(std::move(value)) {}
auto Assignment::to_string(const FormatPolicy &format_policy) const
    -> string_type {
  TODO()
  return {};
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
auto Logical::to_string(const FormatPolicy &format_policy) const
    -> string_type {
  return "(" + left->to_string(format_policy) + " " + op.to_string() + " " +
         right->to_string(format_policy) + ")";
}
auto Call::accept_impl(const ExprVisitor &visitor) const -> expr_result_t {
  return visitor.visit(*this);
}
Get::Get(expr_ptr_t &&object, token_t &&field) : object(object), field(field) {}

auto Call::to_string(const FormatPolicy &format_policy) const -> string_type {
  TODO(...)
  return {};
}
auto Get::accept_impl(const ExprVisitor &visitor) const -> expr_result_t {
  return visitor.visit(*this);
}
auto Get::to_string(const FormatPolicy &format_policy) const -> string_type {
  return "get " + object->to_string(format_policy) + "." +
         field.to_string(format_policy);
}
Set::Set(expr_ptr_t &&object, token_t &&field, expr_ptr_t &&value)
    : object(object), field(field), value(value) {}
auto Set::accept_impl(const ExprVisitor &visitor) const -> expr_result_t {
  return visitor.visit(*this);
}
auto Set::to_string(const auxilia::FormatPolicy &format_policy) const -> string_type {
  TODO()
}
} // namespace accat::loxo::expression
