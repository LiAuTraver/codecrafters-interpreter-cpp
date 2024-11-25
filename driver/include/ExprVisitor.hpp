#pragma once
#include <type_traits>

#include "config.hpp"
#include "loxo_fwd.hpp"
#include "status.hpp"

namespace net::ancillarycat::loxograph::expression {
/// @interface ExprVisitor
class ExprVisitor : public utils::Printable {
public:
  using expr_result_t = std::any;
  using string_view_type = utils::string_view;

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
  auto visit(const DerivedExpr &expr) const {
    return visit_impl(expr);
  }

private:
  virtual expr_result_t visit_impl(const Literal &) const = 0;
  virtual expr_result_t visit_impl(const Unary &) const = 0;
  virtual expr_result_t visit_impl(const Binary &) const = 0;
  virtual expr_result_t visit_impl(const Grouping &) const = 0;
  virtual expr_result_t visit_impl(const IllegalExpr &) const = 0;
};
/// @brief a dummy visitor that does nothing but test compilation
/// @implements ExprVisitor
class DummyVisitor : public ExprVisitor {
public:
  virtual expr_result_t visit_impl(const Literal &expr) const override {
    return {};
  }
  virtual expr_result_t visit_impl(const Unary &expr) const override {
    return {};
  }
  virtual expr_result_t visit_impl(const Binary &expr) const override {
    return {};
  }
  virtual expr_result_t visit_impl(const Grouping &expr) const override {
    return {};
  }
  virtual expr_result_t visit_impl(const IllegalExpr &expr) const override {
    return {};
  }
};
/// @implements ExprVisitor
class LOXOGRAPH_API ExprEvaluator : public ExprVisitor {
public:
  ExprEvaluator() = default;
  virtual ~ExprEvaluator() override = default;

public:
  utils::Status evaluate(const Expr &expr) const;

private:
  virtual expr_result_t visit_impl(const Literal &expr) const override;
  virtual expr_result_t visit_impl(const Unary &expr) const override;
  virtual expr_result_t visit_impl(const Binary &expr) const override;
  virtual expr_result_t visit_impl(const Grouping &expr) const override;
  virtual expr_result_t visit_impl(const IllegalExpr &expr) const override;
  /// @note in Lisp/Scheme, only `#f` is false, everything else is true; we also
  /// make `nil` as false.
  bool is_true_value(const expr_result_t &value) const;
  bool is_deep_equal(const expr_result_t &lhs, const expr_result_t &rhs) const;

private:
  auto to_string_impl(const utils::FormatPolicy &)const -> string_type override;

private:
  const expr_result_t res{};
};
/// @implements ExprVisitor
class LOXOGRAPH_API ASTPrinter : public ExprVisitor {
public:
  using ostream_t = std::ostream;
  using ostringstream_t = std::ostringstream;

public:
  ASTPrinter() = default;
  virtual ~ASTPrinter() override = default;

private:
  virtual expr_result_t visit_impl(const Literal &expr) const override;
  virtual expr_result_t visit_impl(const Unary &expr) const override;
  virtual expr_result_t visit_impl(const Binary &expr) const override;
  virtual expr_result_t visit_impl(const Grouping &expr) const override;
  virtual expr_result_t visit_impl(const IllegalExpr &expr) const override;
  virtual string_type
  to_string_impl(const utils::FormatPolicy &) const override;

private:
  mutable ostringstream_t oss;
  mutable ostringstream_t error_stream;
};
} // namespace net::ancillarycat::loxograph::expression
