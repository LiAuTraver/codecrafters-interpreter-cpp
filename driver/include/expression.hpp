#pragma once

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

/// @namespace accat::loxo::expression
namespace accat::loxo::expression {
/// @interface Expr
class Expr : public auxilia::Printable,
             public std::enable_shared_from_this<Expr> {
public:
  using base_type = Expr;
  using string_type = std::string;
  using ostream_t = std::ostream;
  using ostringstream_t = std::ostringstream;
  using token_t = Token;
  using expr_ptr_t = std::shared_ptr<base_type>;
  using expr_result_t = IVisitor::eval_result_t;

public:
  virtual ~Expr() = default;

public:
  template <typename DerivedVisitor>
    requires std::is_base_of_v<ExprVisitor, DerivedVisitor>
  auto accept(const DerivedVisitor &visitor) const {
    return accept2(visitor);
  }
  auto operator==(const Expr &that) const -> bool {
    return this == &that ||
           // when it comes to overload equiality functions, that probably means
           // the design was flawed.
           (typeid(*this) == typeid(that) && doEqual(*this, that));
  }
  auto operator!=(const Expr &that) const -> bool { return !(*this == that); }

public:
  virtual auto to_string(const auxilia::FormatPolicy &) const
      -> string_type = 0;

private:
  virtual expr_result_t accept2(const ExprVisitor &) const = 0;
  virtual auto doEqual(const Expr &lhs, const Expr &rhs) const -> bool = 0;
};
/// @implements Expr
class Literal : public Expr {

public:
  explicit Literal(token_t &&);

private:
  virtual auto accept2(const ExprVisitor &) const -> expr_result_t override;
  virtual auto doEqual(const Expr &lhs, const Expr &rhs) const
      -> bool override {
    return static_cast<const Literal &>(lhs).literal ==
           static_cast<const Literal &>(rhs).literal;
  }

public:
  virtual auto to_string(const auxilia::FormatPolicy &) const
      -> string_type override;

public:
  token_t literal;
};
/// @implements Expr
class Unary : public Expr {

public:
  explicit Unary(token_t &&, expr_ptr_t &&);
  virtual ~Unary() override = default;

private:
  virtual auto accept2(const ExprVisitor &) const -> expr_result_t override;
  virtual auto doEqual(const Expr &lhs, const Expr &rhs) const
      -> bool override {
    return static_cast<const Unary &>(lhs).op ==
               static_cast<const Unary &>(rhs).op &&
           *static_cast<const Unary &>(lhs).expr ==
               *static_cast<const Unary &>(rhs).expr;
  }

public:
  virtual auto to_string(const auxilia::FormatPolicy &) const
      -> string_type override;

public:
  token_t op;
  expr_ptr_t expr;
};

class Binary : public Expr {

public:
  explicit Binary(token_t &&, expr_ptr_t &&, expr_ptr_t &&);
  virtual ~Binary() = default;

private:
  virtual auto accept2(const ExprVisitor &) const -> expr_result_t override;
  virtual auto doEqual(const Expr &lhs, const Expr &rhs) const
      -> bool override {
    return static_cast<const Binary &>(lhs).op ==
               static_cast<const Binary &>(rhs).op &&
           *static_cast<const Binary &>(lhs).left ==
               *static_cast<const Binary &>(rhs).left &&
           *static_cast<const Binary &>(lhs).right ==
               *static_cast<const Binary &>(rhs).right;
  }

public:
  virtual auto to_string(const auxilia::FormatPolicy &) const
      -> string_type override;

public:
  token_t op;
  expr_ptr_t left;
  expr_ptr_t right;
};

class Variable : public Expr {
public:
  constexpr Variable() = default;
  explicit Variable(token_t &&);
  explicit Variable(const token_t &);
  virtual ~Variable() override = default;

public:
  token_t name;

private:
  auto accept2(const ExprVisitor &) const -> expr_result_t override;

  auto doEqual(const Expr &lhs, const Expr &rhs) const -> bool override {
    return static_cast<const Variable &>(lhs).name ==
           static_cast<const Variable &>(rhs).name;
  }

public:
  auto to_string(const auxilia::FormatPolicy &) const -> string_type override;
};

class Grouping : public Expr {
public:
  explicit Grouping(expr_ptr_t &&);
  virtual ~Grouping() = default;

private:
  virtual auto accept2(const ExprVisitor &) const -> expr_result_t override;
  virtual auto doEqual(const Expr &lhs, const Expr &rhs) const
      -> bool override {
    return *static_cast<const Grouping &>(lhs).expr ==
           *static_cast<const Grouping &>(rhs).expr;
  }

public:
  virtual auto to_string(const auxilia::FormatPolicy &) const
      -> string_type override;

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
  virtual auto accept2(const ExprVisitor &) const -> expr_result_t override;
  virtual auto doEqual(const Expr &lhs, const Expr &rhs) const
      -> bool override {
    return static_cast<const Assignment &>(lhs).name ==
               static_cast<const Assignment &>(rhs).name &&
           *static_cast<const Assignment &>(lhs).value_expr ==
               *static_cast<const Assignment &>(rhs).value_expr;
  }

public:
  virtual auto to_string(const auxilia::FormatPolicy &) const
      -> string_type override;
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
  virtual auto accept2(const ExprVisitor &) const -> expr_result_t override;
  virtual auto doEqual(const Expr &lhs, const Expr &rhs) const
      -> bool override {
    return static_cast<const Logical &>(lhs).op.type ==
               static_cast<const Logical &>(rhs).op.type &&
           *static_cast<const Logical &>(lhs).left ==
               *static_cast<const Logical &>(rhs).left &&
           *static_cast<const Logical &>(lhs).right ==
               *static_cast<const Logical &>(rhs).right;
  }

public:
  virtual auto to_string(const auxilia::FormatPolicy &) const
      -> string_type override;
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
  expr_result_t accept2(const ExprVisitor &) const override;
  auto doEqual(const Expr &lhs, const Expr &rhs) const -> bool override {
    return *static_cast<const Call &>(lhs).callee ==
               *static_cast<const Call &>(rhs).callee &&
           static_cast<const Call &>(lhs).args ==
               static_cast<const Call &>(rhs).args;
  }

public:
  auto to_string(const auxilia::FormatPolicy &) const -> string_type override;
};

class Get : public Expr {
public:
  Get(expr_ptr_t &&, token_t &&);
  virtual ~Get() override = default;

public:
  expr_ptr_t object;
  token_t field;

private:
  auto accept2(const ExprVisitor &) const -> expr_result_t override;
  auto doEqual(const Expr &lhs, const Expr &rhs) const -> bool override {
    return *static_cast<const Get &>(lhs).object ==
               *static_cast<const Get &>(rhs).object &&
           static_cast<const Get &>(lhs).field ==
               static_cast<const Get &>(rhs).field;
  }

public:
  auto to_string(const auxilia::FormatPolicy &) const -> string_type override;
};

class Set : public Expr {
public:
  Set(expr_ptr_t &&, token_t &&, expr_ptr_t &&);

public:
  expr_ptr_t object;
  token_t field;
  expr_ptr_t value;

private:
  auto accept2(const ExprVisitor &) const -> expr_result_t override;
  auto doEqual(const Expr &lhs, const Expr &rhs) const -> bool override {
    return *static_cast<const Set &>(lhs).object ==
               *static_cast<const Set &>(rhs).object &&
           static_cast<const Set &>(lhs).field ==
               static_cast<const Set &>(rhs).field &&
           *static_cast<const Set &>(lhs).value ==
               *static_cast<const Set &>(rhs).value;
  }

public:
  auto to_string(const auxilia::FormatPolicy &) const -> string_type override;
};

class This : public Expr, public auxilia::Viewable {
public:
  This(token_t &&);
  virtual ~This() override = default;

public:
  token_t name; // always `this`

private:
  auto accept2(const ExprVisitor &) const -> expr_result_t override;
  auto doEqual(const Expr &lhs, const Expr &rhs) const -> bool override {
    return static_cast<const This &>(lhs).name ==
           static_cast<const This &>(rhs).name;
  }

public:
  auto to_string(const auxilia::FormatPolicy &) const -> string_type override;
  auto to_string_view(const auxilia::FormatPolicy &) const -> string_view_type;
};

/// @note in the lox language defined by Bob Nystrom, the `super` keyword is
/// used to refer the super class and the `this` keyword refers to this
/// instance, so basically they have different structure in the AST.
class Super : public Expr {
public:
  Super(token_t &&, token_t &&);
  virtual ~Super() override = default;

public:
  token_t name; // always `super`
  token_t method;

private:
  auto accept2(const ExprVisitor &) const -> expr_result_t override;
  auto doEqual(const Expr &lhs, const Expr &rhs) const -> bool override {
    return static_cast<const Super &>(lhs).name ==
               static_cast<const Super &>(rhs).name &&
           static_cast<const Super &>(lhs).method ==
               static_cast<const Super &>(rhs).method;
  }

public:
  auto to_string(const auxilia::FormatPolicy &) const -> string_type override;
};

} // namespace accat::loxo::expression
