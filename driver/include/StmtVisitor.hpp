#ifndef LOXO_STMTVISITOR_HPP
#define LOXO_STMTVISITOR_HPP
#include <variant>
#include <cmath>

#include <net/ancillarycat/utils/Status.hpp>
#include <net/ancillarycat/utils/Variant.hpp>

#include "details/loxo_fwd.hpp"
#include "details/IVisitor.hpp"

#include "Evaluatable.hpp"

namespace net::ancillarycat::loxo::statement {
/// @implements utils::Printable
/// @interface StmtVisitor
class StmtVisitor : virtual public utils::IVisitor {
public:
  template <typename DerivedStmt>
    requires std::is_base_of_v<Stmt, DerivedStmt>
  auto visit(const DerivedStmt &stmt) const {
    return visit_impl(stmt);
  }
  auto execute(const Stmt &stmt) const { return execute_impl(stmt); }

private:
  virtual stmt_result_t visit_impl(const Variable &) const = 0;
  virtual stmt_result_t visit_impl(const Print &) const = 0;
  virtual stmt_result_t visit_impl(const Expression &) const = 0;
  virtual stmt_result_t visit_impl(const Block &) const = 0;
  virtual stmt_result_t visit_impl(const If &) const = 0;
  virtual stmt_result_t visit_impl(const While &) const = 0;
  virtual stmt_result_t visit_impl(const For &) const = 0;
  virtual stmt_result_t visit_impl(const Function &) const = 0;
  virtual stmt_result_t visit_impl(const Return &) const = 0;
  virtual stmt_result_t visit_impl(const IllegalStmt &) const = 0;
  virtual stmt_result_t execute_impl(const Stmt &) const = 0;
};
} // namespace net::ancillarycat::loxo::statement
#endif // LOXO_STMTVISITOR_HPP
