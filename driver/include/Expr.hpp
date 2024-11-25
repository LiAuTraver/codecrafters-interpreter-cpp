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
class Expr : public utils::Printable {
public:
  using ostream_t = std::ostream;
  using ostringstream_t = std::ostringstream;
  using token_t = Token;
  using expr_ptr_t = std::shared_ptr<Expr>;
  using expr_result_t = ExprVisitor::expr_result_t;

public:
  using string_type = std::string;
  virtual ~Expr() = default;
  template <typename DerivedVisitor>
    requires std::is_base_of_v<ExprVisitor, DerivedVisitor>
  auto accept(const DerivedVisitor &visitor) const {
    return accept_impl(visitor);
  }

private:
  virtual expr_result_t accept_impl(const ExprVisitor &visitor) const = 0;

  friend inline ostream_t &operator<<(ostream_t &os, const Expr &expr) {
    return os << expr.to_string();
  }
};
/// @implements Expr
class Literal : public Expr {

public:
  Literal(Token literal) // NOLINT(google-explicit-constructor)
      : literal(std::move(literal)) {}

private:
  virtual expr_result_t accept_impl(const ExprVisitor &visitor) const override;
  virtual string_type
  to_string_impl(const utils::FormatPolicy &) const override;

public:
  Token literal;
};
/// @implements Expr
class Unary : public Expr {

public:
  Unary(Token op, expr_ptr_t expr) : op(std::move(op)), expr(std::move(expr)) {}
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
  Binary(Token op, expr_ptr_t left, expr_ptr_t right)
      : op(std::move(op)), left(std::move(left)), right(std::move(right)) {}
  virtual ~Binary() = default;

private:
  virtual expr_result_t accept_impl(const ExprVisitor &visitor) const override;
  virtual string_type
  to_string_impl(const utils::FormatPolicy &format_policy) const override;

public:
  Token op;
  expr_ptr_t left;
  expr_ptr_t right;
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
class IllegalExpr : public Expr {
public:
  virtual ~IllegalExpr() = default;
  IllegalExpr(Token token, parse_error error)
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
