#include <utility>
#include <cmath>
#include <accat/auxilia/auxilia.hpp>

#include "details/loxo_fwd.hpp"

#include "Evaluatable.hpp"
#include "statement.hpp"

#include "expression.hpp"
#include "StmtVisitor.hpp"

namespace accat::loxo::statement {
using enum auxilia::FormatPolicy;
using auxilia::FormatPolicy;

Stmt::stmt_result_t Variable::accept2(const StmtVisitor &visitor) const {
  return visitor.visit(*this);
}
auto Variable::to_string(const FormatPolicy &format_policy) const
    -> string_type {
  return this->initializer->to_string(format_policy);
}
auto Print::to_string(const FormatPolicy &format_policy) const
    -> string_type {
  return this->value->to_string(format_policy);
}
Stmt::stmt_result_t Print::accept2(const StmtVisitor &visitor) const {
  return visitor.visit(*this);
}
auto Expression::to_string(const FormatPolicy &format_policy) const
    -> string_type {
  return this->expr->to_string(format_policy);
}
Stmt::stmt_result_t Expression::accept2(const StmtVisitor &visitor) const {
  return visitor.visit(*this);
}
auto Block::to_string(const FormatPolicy &format_policy) const
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
Stmt::stmt_result_t Block::accept2(const StmtVisitor &visitor) const {
  return visitor.visit(*this);
}

Stmt::stmt_result_t If::accept2(const StmtVisitor &visitor) const {
  return visitor.visit(*this);
}
auto While::to_string(const FormatPolicy &format_policy) const
    -> string_type {
  return "while (" + this->condition->to_string(format_policy) + ") " +
         this->body->to_string(format_policy);
}
auto While::accept2(const StmtVisitor &visitor) const -> stmt_result_t {
  return visitor.visit(*this);
}
auto If::to_string(const FormatPolicy &format_policy) const
    -> string_type {
  string_type result = "if (" + this->condition->to_string(format_policy) +
                       ") " + this->then_branch->to_string(format_policy);
  if (this->else_branch) {
    result += " else " + this->else_branch->to_string(format_policy);
  }
  return result;
}
auto Function::to_string(const FormatPolicy &format_policy) const
    -> string_type {
  string_type result = "function " + this->name.to_string(format_policy) + "(";
  for (const auto &param : this->parameters) {
    result += param.to_string(format_policy) + ", ";
  }
  if (!this->parameters.empty()) {
    result.pop_back();
    result.pop_back();
  }
  result.append(") { ... }");
  return result;
}
auto Function::accept2(const StmtVisitor &visitor) const -> stmt_result_t {
  return visitor.visit(*this);
}
auto Class::to_string(const auxilia::FormatPolicy &format_policy) const -> string_type {
  return "class "s.append(this->name.to_string(format_policy)).append(" { ... }");
}
auto Class::accept2(const StmtVisitor &visitor) const -> stmt_result_t {
  return visitor.visit(*this);
}
auto Return::to_string(const FormatPolicy &format_policy) const
    -> string_type {
  return "return "s.append(this->value->to_string(format_policy));
}
auto Return::accept2(const StmtVisitor &visitor) const -> stmt_result_t {
  return visitor.visit(*this);
}
auto For::to_string(const FormatPolicy &format_policy) const
    -> string_type {
  auto result = "for ("s;
  if (this->initializer) {
    result += this->initializer->to_string(format_policy);
  }
  result += "; ";
  if (this->condition) {
    result += this->condition->to_string(format_policy);
  }
  result += "; ";
  if (this->increment) {
    result += this->increment->to_string(format_policy);
  }
  contract_assert(!!this->body, "for loop body is null; shouldn't happen")
  result += ") " + this->body->to_string(format_policy);
  return result;
}
Stmt::stmt_result_t For::accept2(const StmtVisitor &visitor) const {
  return visitor.visit(*this);
}

} // namespace accat::loxo::statement
