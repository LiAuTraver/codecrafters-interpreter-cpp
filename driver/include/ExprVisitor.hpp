#pragma once

#include <type_traits>
#include <variant>

#include <accat/auxilia/auxilia.hpp>

#include "details/loxo_fwd.hpp"

#include "details/IVisitor.hpp"
#include "Evaluatable.hpp"
namespace accat::loxo::expression {
/// @interface ExprVisitor
class ExprVisitor : virtual public IVisitor {
public:
  virtual ~ExprVisitor() = default;

public:
  // clang-format off
  /// @brief Visit the expression
  /// @attention tbh i don't really like the idea of making a virtual
  ///      function private, but coreguidelines says it's a good practice.
  /// @note see <a href="https://stackoverflow.com/questions/2170688/private-virtual-method-in-c">here</a>
  // clang-format on
  template <typename DerivedExpr>
    requires std::is_base_of_v<Expr, DerivedExpr>
  auto visit(const DerivedExpr &expr) const {
    // workaround
    return const_cast<ExprVisitor *>(this)->visit_impl(expr);
  }
  auto evaluate(const Expr &expr) const {
    // workaround
    return const_cast<ExprVisitor *>(this)->evaluate_impl(expr);
  }
  auto get_result() const { return get_result_impl(); }

private:
  virtual eval_result_t visit_impl(const Literal &) = 0;
  virtual eval_result_t visit_impl(const Unary &) = 0;
  virtual eval_result_t visit_impl(const Binary &) = 0;
  virtual eval_result_t visit_impl(const Grouping &) = 0;
  virtual eval_result_t visit_impl(const Variable &) = 0;
  virtual eval_result_t visit_impl(const Assignment &) = 0;
  virtual eval_result_t visit_impl(const Logical &) = 0;
  virtual eval_result_t visit_impl(const Call &) = 0;
  virtual eval_result_t visit_impl(const Get &) = 0;
  virtual eval_result_t visit_impl(const Set &) = 0;
  virtual eval_result_t visit_impl(const This&) = 0;

private:
  virtual eval_result_t evaluate_impl(const Expr &) = 0;
  virtual eval_result_t get_result_impl() const = 0;
};
} // namespace accat::loxo::expression
