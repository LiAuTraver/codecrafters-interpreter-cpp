#include <utility>
#include <cmath>

#include "config.hpp"
#include "utils.hpp"
#include "loxo_fwd.hpp"

#include "Evaluatable.hpp"
#include "statement.hpp"

#include "expression.hpp"

namespace net::ancillarycat::loxograph::statement {
Stmt::stmt_result_t Variable::accept_impl(const StmtVisitor &visitor) const {
  return visitor.visit(*this);
}
auto Variable::to_string_impl(const utils::FormatPolicy &format_policy) const
    -> string_type {
  return this->initializer->to_string(format_policy);
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
auto Block::to_string_impl(const utils::FormatPolicy &format_policy) const
    -> string_type {
  if (statements.empty())
    return "{<empty block>}";

  string_type result;
  result += "{\n";
  for (const auto &stmt : this->statements) {
    result += stmt->to_string(format_policy);
  }
  result += "}\n";
  return result;
}
Stmt::stmt_result_t Block::accept_impl(const StmtVisitor &visitor) const {
  return visitor.visit(*this);
}

Stmt::stmt_result_t If::accept_impl(const StmtVisitor &visitor) const {
  return visitor.visit(*this);
}
auto While::to_string_impl(const utils::FormatPolicy &format_policy) const
    -> string_type {
  return "while (" + this->condition->to_string(format_policy) + ") " +
         this->body->to_string(format_policy);
}
auto While::accept_impl(const StmtVisitor &visitor) const -> stmt_result_t {
  return visitor.visit(*this);
}
auto If::to_string_impl(const utils::FormatPolicy &format_policy) const
    -> string_type {
  string_type result = "if (" + this->condition->to_string(format_policy) +
                       ") " + this->then_branch->to_string(format_policy);
  if (this->else_branch) {
    result += " else " + this->else_branch->to_string(format_policy);
  }
  return result;
}

auto For::to_string_impl(const utils::FormatPolicy &format_policy) const
    -> string_type {
  return "for (" + this->initializer->to_string(format_policy) + "; " +
         this->condition->to_string(format_policy) + "; " +
         this->increment->to_string(format_policy) + ") " +
         this->body->to_string(format_policy);
}
Stmt::stmt_result_t For::accept_impl(const StmtVisitor &visitor) const {
  return visitor.visit(*this);
}
auto IllegalStmt::to_string_impl(const utils::FormatPolicy &) const
    -> string_type {
  return message;
}
auto IllegalStmt::to_string_view_impl(const utils::FormatPolicy &) const
    -> string_view_type {
  return message;
}
Stmt::stmt_result_t IllegalStmt::accept_impl(const StmtVisitor &visitor) const {
  return visitor.visit(*this);
}
} // namespace net::ancillarycat::loxograph::statement
