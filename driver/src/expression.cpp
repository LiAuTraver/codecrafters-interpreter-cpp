#include <string>

#include "accat/auxilia/details/format.hpp"
#include "details/lox_fwd.hpp"

#include "expression.hpp"
#include "ExprVisitor.hpp"
#include "Evaluatable.hpp"

namespace accat::lox::expression {
using auxilia::FormatPolicy;
using enum auxilia::FormatPolicy;

Literal::Literal(token_t &&literal) : literal(std::move(literal)) {}
Expr::expr_result_t Literal::accept2(const ExprVisitor &visitor) const {
  return visitor.visit(*this);
}
Expr::string_type Literal::to_string(const FormatPolicy &) const {
  return literal.to_string(kDetailed);
}
Unary::Unary(token_t &&op, expr_ptr_t &&expr)
    : op(std::move(op)), expr(std::move(expr)) {}
Expr::expr_result_t Unary::accept2(const ExprVisitor &visitor) const {
  return visitor.visit(*this);
}
Expr::string_type Unary::to_string(const FormatPolicy &format_policy) const {
  return "(" + op.to_string(kDetailed) + " " + expr->to_string(format_policy) +
         ")";
}
Binary::Binary(token_t &&op, expr_ptr_t &&left, expr_ptr_t &&right)
    : op(std::move(op)), left(std::move(left)), right(std::move(right)) {}
Expr::expr_result_t Binary::accept2(const ExprVisitor &visitor) const {
  return visitor.visit(*this);
}
Expr::string_type Binary::to_string(const FormatPolicy &format_policy) const {
  return "(" + op.to_string(kDetailed) + " " + left->to_string(format_policy) +
         " " + right->to_string(format_policy) + ")";
}
Variable::Variable(token_t &&name) : name(std::move(name)) {}
Variable::Variable(const token_t &name) : name(name) {}
Expr::expr_result_t Variable::accept2(const ExprVisitor &visitor) const {
  return visitor.visit(*this);
}
auto Variable::to_string(const FormatPolicy &format_policy) const
    -> string_type {
  return name.to_string(kDetailed);
}
Grouping::Grouping(expr_ptr_t &&expr) : expr(std::move(expr)) {}
Expr::expr_result_t Grouping::accept2(const ExprVisitor &visitor) const {
  return visitor.visit(*this);
}
Expr::string_type Grouping::to_string(const FormatPolicy &format_policy) const {
  /// strange print format, but codecrafter's test needs this.
  return "(group " + expr->to_string(format_policy) + ")";
}
Expr::expr_result_t Assignment::accept2(const ExprVisitor &visitor) const {
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
Expr::expr_result_t Logical::accept2(const ExprVisitor &visitor) const {
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
auto Call::accept2(const ExprVisitor &visitor) const -> expr_result_t {
  return visitor.visit(*this);
}
Get::Get(expr_ptr_t &&object, token_t &&field) : object(object), field(field) {}

auto Call::to_string(const FormatPolicy &format_policy) const -> string_type {
  TODO(...)
  return {};
}
auto Get::accept2(const ExprVisitor &visitor) const -> expr_result_t {
  return visitor.visit(*this);
}
auto Get::to_string(const FormatPolicy &format_policy) const -> string_type {
  return "get " + object->to_string(format_policy) + "." +
         field.to_string(format_policy);
}
Set::Set(expr_ptr_t &&object, token_t &&field, expr_ptr_t &&value)
    : object(object), field(field), value(value) {}
auto Set::accept2(const ExprVisitor &visitor) const -> expr_result_t {
  return visitor.visit(*this);
}
auto Set::to_string(const FormatPolicy &format_policy) const -> string_type {
  TODO()
}
This::This(token_t &&name) : name(std::move(name)) {}
auto This::accept2(const ExprVisitor &visitor) const -> expr_result_t {
  return visitor.visit(*this);
}
auto This::to_string(const FormatPolicy &format_policy) const -> string_type {
  return "this";
}
auto This::to_string_view(const FormatPolicy &format_policy) const
    -> string_view_type {
  return "this";
}
Super::Super(token_t &&name, token_t &&method)
    : name(std::move(name)), method(std::move(method)) {}
auto Super::accept2(const ExprVisitor &visitor) const -> expr_result_t {
  return visitor.visit(*this);
}
auto Super::to_string(const auxilia::FormatPolicy &format_policy) const -> string_type {
  return "super."s.append(method.to_string(format_policy));
}
} // namespace accat::lox::expression
