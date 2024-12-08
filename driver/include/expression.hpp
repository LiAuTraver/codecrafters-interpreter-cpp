#pragma once

#include <any>
#include <concepts>
#include <cstddef>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "details/loxo_fwd.hpp"

#include "details/IVisitor.hpp"
#include "Token.hpp"
#include "parse_error.hpp"

/// @namespace net::ancillarycat::loxo::expression
namespace net::ancillarycat::loxo::expression {
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
  using expr_result_t = utils::IVisitor::eval_result_t;

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
  explicit Literal(token_t &&);

private:
  virtual auto accept_impl(const ExprVisitor &) const -> expr_result_t override;
  virtual auto
  to_string_impl(const utils::FormatPolicy &) const -> string_type override;

public:
  token_t literal;
};
/// @implements Expr
class Unary : public Expr {

public:
  explicit Unary(token_t &&, expr_ptr_t &&);
  virtual ~Unary() override = default;

private:
  virtual auto accept_impl(const ExprVisitor &) const -> expr_result_t override;
  virtual auto
  to_string_impl(const utils::FormatPolicy &) const -> string_type override;

public:
  token_t op;
  expr_ptr_t expr;
};

class Binary : public Expr {

public:
  explicit Binary(token_t &&, expr_ptr_t &&, expr_ptr_t &&);
  virtual ~Binary() = default;

private:
  virtual auto accept_impl(const ExprVisitor &) const -> expr_result_t override;
  virtual auto
  to_string_impl(const utils::FormatPolicy &) const -> string_type override;

public:
  token_t op;
  expr_ptr_t left;
  expr_ptr_t right;
};

class Variable : public Expr {
public:
  explicit Variable(token_t &&);
  virtual ~Variable() override = default;

public:
  token_t name;

private:
  auto accept_impl(const ExprVisitor &) const -> expr_result_t override;
  auto
  to_string_impl(const utils::FormatPolicy &) const -> string_type override;
};

class Grouping : public Expr {
public:
  explicit Grouping(expr_ptr_t &&);
  virtual ~Grouping() = default;

private:
  virtual auto accept_impl(const ExprVisitor &) const -> expr_result_t override;
  virtual auto
  to_string_impl(const utils::FormatPolicy &) const -> string_type override;

public:
  expr_ptr_t expr;
};
/// @implements Expr
class Assignment : public Expr {
public:
  constexpr Assignment() = default;
  explicit Assignment(token_t &&, expr_ptr_t &&);
  virtual ~Assignment() override = default;

public:
  token_t name;
  expr_ptr_t value_expr;

private:
  virtual auto accept_impl(const ExprVisitor &) const -> expr_result_t override;
  virtual auto
  to_string_impl(const utils::FormatPolicy &) const -> string_type override;
};
/// @implements Expr
class Logical : public Expr {
public:
  constexpr Logical() = default;
  explicit Logical(token_t &&, expr_ptr_t &&, expr_ptr_t &&);
  virtual ~Logical() override = default;

public:
  token_t op;
  expr_ptr_t left;
  expr_ptr_t right;

private:
  virtual auto
  to_string_impl(const utils::FormatPolicy &) const -> string_type override;
  virtual auto accept_impl(const ExprVisitor &) const -> expr_result_t override;
};
class Call : public Expr {
public:
  explicit Call(expr_ptr_t &&, token_t &&, std::vector<expr_ptr_t> &&);
  virtual ~Call() override = default;

public:
  expr_ptr_t callee;
  token_t paren;
  std::vector<expr_ptr_t> args;

private:
  auto
  to_string_impl(const utils::FormatPolicy &) const -> string_type override;
  expr_result_t accept_impl(const ExprVisitor &) const override;
};

} // namespace net::ancillarycat::loxo::expression
