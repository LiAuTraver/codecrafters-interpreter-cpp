#pragma once
#include <type_traits>
#include <variant>

#include <net/ancillarycat/utils/Variant.hpp>
#include <net/ancillarycat/utils/status.hpp>

#include "loxo_fwd.hpp"

#include "IVisitor.hpp"
#include "Evaluatable.hpp"
namespace net::ancillarycat::loxo::expression {
/// @interface ExprVisitor
class ExprVisitor : virtual public utils::VisitorBase {
public:
  virtual ~ExprVisitor() = default;

public:
  // clang-format off
  /// @brief Visit the expression
  /// @attention tbh i don't really like the idea of making a virtual
  ///      function private, but coreguidelines says it's a good practice.
  /// @note  why make virtual function private? see <a href="https://stackoverflow.com/questions/2170688/private-virtual-method-in-c">
  ///      here</a>
  // clang-format on
  template <typename DerivedExpr>
    requires std::is_base_of_v<Expr, DerivedExpr>
  auto visit(const DerivedExpr &expr) const {
    return visit_impl(expr);
  }
  auto evaluate(const Expr &expr) const { return evaluate_impl(expr); }
  auto get_result() const { return get_result_impl(); }

private:
  virtual eval_result_t visit_impl(const Literal &) const = 0;
  virtual eval_result_t visit_impl(const Unary &) const = 0;
  virtual eval_result_t visit_impl(const Binary &) const = 0;
  virtual eval_result_t visit_impl(const Grouping &) const = 0;
  virtual eval_result_t visit_impl(const Variable &) const = 0;
  virtual eval_result_t visit_impl(const Assignment &) const = 0;
  virtual eval_result_t visit_impl(const Logical &) const = 0;
  virtual eval_result_t visit_impl(const Call &) const = 0;
  virtual eval_result_t visit_impl(const IllegalExpr &) const = 0;

private:
  virtual stmt_result_t evaluate_impl(const Expr &) const = 0;
  virtual eval_result_t get_result_impl() const = 0;
};
/// @implements ExprVisitor
class LOXO_API ASTPrinter : public ExprVisitor, public utils::Viewable {
public:
  using ostream_t = std::ostream;
  using ostringstream_t = std::ostringstream;

public:
  ASTPrinter() = default;
  virtual ~ASTPrinter() override = default;

private:
  virtual eval_result_t visit_impl(const Literal &) const override;
  virtual eval_result_t visit_impl(const Unary &) const override;
  virtual eval_result_t visit_impl(const Binary &) const override;
  virtual eval_result_t visit_impl(const Grouping &) const override;
  virtual eval_result_t visit_impl(const Variable &) const override;
  virtual eval_result_t visit_impl(const Assignment &) const override;
  virtual eval_result_t visit_impl(const Logical &) const override;
  virtual eval_result_t visit_impl(const Call &) const override;
  virtual eval_result_t visit_impl(const IllegalExpr &) const override;
  virtual stmt_result_t evaluate_impl(const Expr &) const override;
  virtual string_type
  to_string_impl(const utils::FormatPolicy &) const override;
  auto to_string_view_impl(const utils::FormatPolicy &) const
      -> utils::Viewable::string_view_type override;
  eval_result_t get_result_impl() const override;

private:
  eval_result_t res{utils::Monostate{}};
  mutable ostringstream_t oss;
  mutable ostringstream_t error_stream;
};
} // namespace net::ancillarycat::loxo::expression
