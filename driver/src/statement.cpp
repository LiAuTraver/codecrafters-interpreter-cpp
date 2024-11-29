#include <utility>

#include "config.hpp"
#include "fmt.hpp"
#include "loxo_fwd.hpp"

#include "Evaluatable.hpp"
#include "statement.hpp"

#include "expression.hpp"

namespace net::ancillarycat::loxograph::statement {
Stmt::stmt_result_t Variable::accept_impl(const StmtVisitor &) const {
  return {std::monostate{}};
}
auto Variable::to_string_impl(const utils::FormatPolicy &format_policy) const
    -> string_type {
  return {};
}
auto Print::to_string_impl(const utils::FormatPolicy &format_policy) const
    -> string_type {
    return this->value->to_string(format_policy);
}
Stmt::stmt_result_t Print::accept_impl(const StmtVisitor &visitor) const {
  // TODO
  auto _ = visitor.visit(*this);
  return {};
}
auto Expression::to_string_impl(const utils::FormatPolicy &format_policy) const
    -> string_type {
  return this->expr->to_string(format_policy);
}
Stmt::stmt_result_t Expression::accept_impl(const StmtVisitor &visitor) const {
  // TODO
  auto _ = visitor.visit(*this);
  return {};
}
} // namespace net::ancillarycat::loxograph::statement
