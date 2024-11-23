#pragma once

#include <concepts>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "config.hpp"
#include "loxo_fwd.hpp"
#include "parse_error.hpp"
#include "Token.hpp"

namespace net::ancillarycat::loxograph {
/// @interface ExprVisitor
class ExprVisitor {
public:
  virtual ~ExprVisitor() = default;
  // clang-format off
  /// @brief Visit the expression
  /// @attention tbh i don't really like the idea of making a virtual
  ///      function private, but coreguidelines says it's a good practice.
  /// @note  why make virtual function private? see <a href="https://stackoverflow.com/questions/2170688/private-virtual-method-in-c">here</a>
  // clang-format on
  template <typename DerivedExpr>
    requires std::is_base_of_v<Expr, DerivedExpr>
  auto visit(const DerivedExpr &expr) {
    return visit_impl(expr);
  }

private:
  virtual void visit_impl(const Literal &expr) = 0;
  virtual void visit_impl(const Unary &expr) = 0;
  virtual void visit_impl(const Binary &expr) = 0;
  virtual void visit_impl(const Grouping &expr) = 0;
  virtual void visit_impl(const IllegalExpr &expr) = 0;
};
/// @interface Expr
class Expr {
public:
  using ostream_t = std::ostream;
  using ostringstream_t = std::ostringstream;
  using token_t = Token;

public:
  using string_type = std::string;
  virtual ~Expr() = default;
  template <typename DerivedVisitor>
    requires std::is_base_of_v<ExprVisitor, DerivedVisitor>
  auto accept(DerivedVisitor &visitor) {
    return accept_impl(visitor);
  }
  auto to_string() const { return to_string_impl(); }

private:
  virtual void accept_impl(ExprVisitor &visitor) = 0;
  virtual std::string to_string_impl() const = 0;

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
  virtual void accept_impl(ExprVisitor &visitor) override {
    visitor.visit(*this);
  }
  virtual string_type to_string_impl() const override {
    return literal.to_string(Token::kTokenOnly);
  }

public:
  Token literal;
};

class Unary : public Expr {

public:
  Unary(Token op, std::shared_ptr<Expr> expr)
      : op(std::move(op)), expr(std::move(expr)) {}
  virtual ~Unary() override = default;

private:
  virtual void accept_impl(ExprVisitor &visitor) override {
    visitor.visit(*this);
  }
  virtual string_type to_string_impl() const override {
    return "(" + op.to_string(Token::kTokenOnly) + " " + expr->to_string() +
           ")";
  }

public:
  token_t op;
  std::shared_ptr<Expr> expr;
};

class Binary : public Expr {

public:
  Binary(Token op, std::shared_ptr<Expr> left, std::shared_ptr<Expr> right)
      : op(std::move(op)), left(std::move(left)), right(std::move(right)) {}
  virtual ~Binary() = default;

private:
  virtual void accept_impl(ExprVisitor &visitor) override {
    visitor.visit(*this);
  }
  virtual string_type to_string_impl() const override {
    return "(" + op.to_string(Token::kTokenOnly) + " " + left->to_string() +
           " " + right->to_string() + ")";
  }

public:
  Token op;
  std::shared_ptr<Expr> left;
  std::shared_ptr<Expr> right;
};

class Grouping : public Expr {

public:
  Grouping(std::shared_ptr<Expr> expr) : expr(std::move(expr)) {} // NOLINT
  virtual ~Grouping() = default;

private:
  virtual void accept_impl(ExprVisitor &visitor) override {
    visitor.visit(*this);
  }
  virtual string_type to_string_impl() const override {
    /// strange print format, but codecrafter's test needs this.
    return "(group " + expr->to_string() + ")";
  }

public:
  std::shared_ptr<Expr> expr;
};

/// @implements Expr
class IllegalExpr : public Expr {
public:
  virtual ~IllegalExpr() = default;
  IllegalExpr(Token token, parse_error error)
      : token(std::move(token)), error(std::move(error)) {}

private:
  virtual void accept_impl(ExprVisitor &visitor) override {
    visitor.visit(*this);
  }
  virtual std::string to_string_impl() const override {
    return utils::format("[line {}] Error at '{}': {}",
                         token.line,
                         token.to_string(Token::kTokenOnly),
                         error.message());
  }

public:
  token_t token;
  parse_error error;
  // std::shared_ptr<IllegalExpr> next = nullptr;
};
class DummyVisitor : public ExprVisitor {
public:
  virtual void visit_impl(const Literal &expr) override {}
  virtual void visit_impl(const Unary &expr) override {}
  virtual void visit_impl(const Binary &expr) override {}
  virtual void visit_impl(const Grouping &expr) override {}
  virtual void visit_impl(const IllegalExpr &expr) override {}
};

class ASTPrinter : public ExprVisitor {
public:
  using ostream_t = std::ostream;
  using ostringstream_t = std::ostringstream;

public:
  ASTPrinter() = default;
  virtual ~ASTPrinter() override = default;

public:
  auto to_string() const -> std::string { return oss.str(); }

private:
  virtual void visit_impl(const Literal &expr) override {
    dbg(info, "Literal: {}", expr.to_string());
    oss << expr << std::endl;
  }
  virtual void visit_impl(const Unary &expr) override {
    dbg(info, "Unary: {}", expr.to_string());
    oss << expr << std::endl;
  }
  virtual void visit_impl(const Binary &expr) override {
    dbg(info, "Binary: {}", expr.to_string());
    oss << expr << std::endl;
  }
  virtual void visit_impl(const Grouping &expr) override {
    dbg(info, "Grouping: {}", expr.to_string());
    oss << expr << std::endl;
  }
  virtual void visit_impl(const IllegalExpr &expr) override {
    dbg(info, "IllegalExpr: {}", expr.to_string());
    error_stream << expr << std::endl;
  }

private:
  ostringstream_t oss;
  ostringstream_t error_stream;
};
} // namespace net::ancillarycat::loxograph

#ifdef LOXOGRAPH_USE_FMT_FORMAT
template <> struct fmt::formatter<net::ancillarycat::loxograph::Expr> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  auto format(const net::ancillarycat::loxograph::Expr &expr,
              FormatContext &ctx) {
    return format_to(ctx.out(), "{}", expr.to_string());
  }
};
template <> struct fmt::formatter<net::ancillarycat::loxograph::ExprVisitor> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  auto format(const net::ancillarycat::loxograph::ExprVisitor &visitor,
              FormatContext &ctx) {
    return format_to(ctx.out(), "{}", "ExprVisitor");
  }
};
#endif
