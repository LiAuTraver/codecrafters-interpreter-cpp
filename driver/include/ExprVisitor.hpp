#ifndef LOXO_EXPRVISITOR_HPP
#define LOXO_EXPRVISITOR_HPP
#include <type_traits>
#include <variant>

#include <net/ancillarycat/utils/Variant.hpp>
#include <net/ancillarycat/utils/Status.hpp>

#include "details/loxo_fwd.hpp"

#include "details/IVisitor.hpp"
#include "Evaluatable.hpp"
namespace net::ancillarycat::loxo::expression {
/// @interface ExprVisitor
class ExprVisitor : virtual public utils::IVisitor {
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
} // namespace net::ancillarycat::loxo::expression
#endif // LOXO_EXPRVISITOR_HPP
