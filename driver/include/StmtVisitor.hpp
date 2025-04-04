#ifndef LOXO_STMTVISITOR_HPP
#define LOXO_STMTVISITOR_HPP
#include <variant>
#include <cmath>

#include <accat/auxilia/auxilia.hpp>

#include "details/loxo_fwd.hpp"
#include "details/IVisitor.hpp"

#include "Evaluatable.hpp"

namespace accat::loxo::statement {
/// @implements auxilia::Printable
/// @interface StmtVisitor
class StmtVisitor : virtual public IVisitor {
public:
  template <typename DerivedStmt>
    requires std::is_base_of_v<Stmt, DerivedStmt>
  auto visit(const DerivedStmt &stmt) const {
    // workaround
    return const_cast<StmtVisitor *>(this)->visit_impl(stmt);
  }
  auto execute(const Stmt &stmt) const {
    // workaround
    return const_cast<StmtVisitor *>(this)->execute_impl(stmt);
  }

private:
  virtual eval_result_t visit_impl(const Variable &) = 0;
  virtual eval_result_t visit_impl(const Print &) = 0;
  virtual eval_result_t visit_impl(const Expression &) = 0;
  virtual eval_result_t visit_impl(const Block &) = 0;
  virtual eval_result_t visit_impl(const If &) = 0;
  virtual eval_result_t visit_impl(const While &) = 0;
  virtual eval_result_t visit_impl(const For &) = 0;
  virtual eval_result_t visit_impl(const Function &) = 0;
  virtual eval_result_t visit_impl(const Class &) = 0;
  virtual eval_result_t visit_impl(const Return &) = 0;
  virtual eval_result_t execute_impl(const Stmt &) = 0;

public:
  virtual ~StmtVisitor() = default;
};
} // namespace accat::loxo::statement
#endif // LOXO_STMTVISITOR_HPP
