#pragma once

#include <type_traits>
#include <variant>

#include <accat/auxilia/auxilia.hpp>

#include "details/lox_fwd.hpp"

#include "details/IVisitor.hpp"
#include "Evaluatable.hpp"
namespace accat::lox::expression {
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
    return const_cast<ExprVisitor *>(this)->visit2(expr);
  }
  auto evaluate(const Expr &expr) const {
    // workaround
    return const_cast<ExprVisitor *>(this)->evaluate4(expr);
  }
  auto get_result() const { return get_result_impl(); }

private:
  virtual auto visit2(const Literal &) -> eval_result_t = 0;
  virtual auto visit2(const Unary &) -> eval_result_t = 0;
  virtual auto visit2(const Binary &) -> eval_result_t = 0;
  virtual auto visit2(const Grouping &) -> eval_result_t = 0;
  virtual auto visit2(const Variable &) -> eval_result_t = 0;
  virtual auto visit2(const Assignment &) -> eval_result_t = 0;
  virtual auto visit2(const Logical &) -> eval_result_t = 0;
  virtual auto visit2(const Call &) -> eval_result_t = 0;
  virtual auto visit2(const Get &) -> eval_result_t = 0;
  virtual auto visit2(const Set &) -> eval_result_t = 0;
  virtual auto visit2(const This &) -> eval_result_t = 0;
  virtual auto visit2(const Super &) -> eval_result_t = 0;

private:
  virtual auto evaluate4(const Expr &) -> eval_result_t = 0;
  virtual auto get_result_impl() const -> eval_result_t = 0;
};
} // namespace accat::lox::expression
