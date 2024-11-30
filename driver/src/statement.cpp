#include <utility>

#include "config.hpp"
#include "fmt.hpp"
#include "loxo_fwd.hpp"

#include "Evaluatable.hpp"
#include "statement.hpp"

#include "expression.hpp"

namespace net::ancillarycat::loxograph::statement {
Stmt::stmt_result_t Variable::accept_impl(const StmtVisitor &) const {
  return utils::NotImplementedError("Variable::accept_impl");
}
auto Variable::to_string_impl(const utils::FormatPolicy &format_policy) const
    -> string_type {
  return "not implemented"s;
}
auto Print::to_string_impl(const utils::FormatPolicy &format_policy) const
    -> string_type {
  return this->value->to_string(format_policy);
}
Stmt::stmt_result_t Print::accept_impl(const StmtVisitor &visitor) const {
  return visitor.visit(*this);
}
auto Expression::to_string_impl(const utils::FormatPolicy &format_policy) const
    -> string_type {
  return this->expr->to_string(format_policy);
}
Stmt::stmt_result_t Expression::accept_impl(const StmtVisitor &visitor) const {
  return visitor.visit(*this);
}
} // namespace net::ancillarycat::loxograph::statement
