#pragma once

#include <any>
#include <concepts>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "config.hpp"
#include "ExprVisitor.hpp"
#include "loxo_fwd.hpp"
#include "parse_error.hpp"
#include "Token.hpp"

/// @namespace net::ancillarycat::loxograph::expression
namespace net::ancillarycat::loxograph::expression {
/// @interface Expr
class Expr : public utils::Printable,
             public std::enable_shared_from_this<Expr> {
public:
  using base_type = Expr;
  using string_type = std::string;
  using ostream_t = std::ostream;
  using ostringstream_t = std::ostringstream;
  using token_t = Token;
  using expr_ptr_t = std::shared_ptr<base_type>;
  using expr_result_t = ExprVisitor::eval_result_t;

public:
  virtual ~Expr() = default;

public:
  template <typename DerivedVisitor>
    requires std::is_base_of_v<ExprVisitor, DerivedVisitor>
  auto accept(const DerivedVisitor &visitor) const {
    return accept_impl(visitor);
  }

private:
  virtual expr_result_t accept_impl(const ExprVisitor &) const = 0;
};
/// @implements Expr
class Literal : public Expr {

public:
  Literal(token_t literal) // NOLINT(google-explicit-constructor)
      : literal(std::move(literal)) {}

private:
  virtual expr_result_t accept_impl(const ExprVisitor &visitor) const override;
  virtual string_type
  to_string_impl(const utils::FormatPolicy &) const override;

public:
  token_t literal;
};
/// @implements Expr
class Unary : public Expr {

public:
  Unary(token_t op, expr_ptr_t expr)
      : op(std::move(op)), expr(std::move(expr)) {}
  virtual ~Unary() override = default;

private:
  virtual expr_result_t accept_impl(const ExprVisitor &visitor) const override;
  virtual string_type
  to_string_impl(const utils::FormatPolicy &format_policy) const override;

public:
  token_t op;
  expr_ptr_t expr;
};

class Binary : public Expr {

public:
  Binary(token_t op, expr_ptr_t left, expr_ptr_t right)
      : op(std::move(op)), left(std::move(left)), right(std::move(right)) {}
  virtual ~Binary() = default;

private:
  virtual expr_result_t accept_impl(const ExprVisitor &visitor) const override;
  virtual string_type
  to_string_impl(const utils::FormatPolicy &format_policy) const override;

public:
  token_t op;
  expr_ptr_t left;
  expr_ptr_t right;
};
class Variable : public Expr {
public:
  Variable(token_t name) : name(std::move(name)) {}
  virtual ~Variable() override = default;

public:
  token_t name;

private:
  expr_result_t accept_impl(const ExprVisitor &visitor) const override;
  auto to_string_impl(const utils::FormatPolicy &format_policy) const
      -> string_type override;
};
class Grouping : public Expr {

public:
  Grouping(expr_ptr_t expr) : expr(std::move(expr)) {} // NOLINT
  virtual ~Grouping() = default;

private:
  virtual expr_result_t accept_impl(const ExprVisitor &visitor) const override;
  virtual string_type
  to_string_impl(const utils::FormatPolicy &format_policy) const override;

public:
  expr_ptr_t expr;
};
/// @implements Expr
class Assignment : public Expr {
public:
  constexpr Assignment() = default;
  virtual ~Assignment() override = default;
  Assignment(token_t name, expr_ptr_t value)
      : name(std::move(name)), value_expr(std::move(value)) {}

public:
  token_t name;
  expr_ptr_t value_expr;

private:
  auto to_string_impl(const utils::FormatPolicy &) const
      -> string_type override;
  expr_result_t accept_impl(const ExprVisitor &) const override;
};
/// @implements Expr
class Logical : public Expr {
public:
  constexpr Logical() = default;
  explicit Logical(token_t &&op, expr_ptr_t &&left, expr_ptr_t &&right)
      : op(std::move(op)), left(std::move(left)), right(std::move(right)) {}
  virtual ~Logical() override = default;

public:
  token_t op{};
  expr_ptr_t left{nullptr};
  expr_ptr_t right{nullptr};

private:
  auto to_string_impl(const utils::FormatPolicy &) const
      -> string_type override;
  expr_result_t accept_impl(const ExprVisitor &) const override;
};
/// @implements Expr
class IllegalExpr : public Expr {
public:
  constexpr IllegalExpr() = default;
  virtual ~IllegalExpr() override = default;
  IllegalExpr(token_t token, parse_error error)
      : token(std::move(token)), error(std::move(error)) {}

private:
  virtual expr_result_t accept_impl(const ExprVisitor &visitor) const override;
  virtual std::string
  to_string_impl(const utils::FormatPolicy &format_policy) const override;

public:
  token_t token;
  parse_error error;
  // std::shared_ptr<IllegalExpr> next = nullptr;
};
} // namespace net::ancillarycat::loxograph::expression
