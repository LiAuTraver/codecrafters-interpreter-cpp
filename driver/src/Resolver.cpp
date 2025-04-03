#include "Resolver.hpp"

#include <algorithm>
#include <iterator>
#include <string>

#include "Token.hpp"
#include "expression.hpp"
#include "statement.hpp"
#include "interpreter.hpp"

namespace accat::loxo {
using enum auxilia::FormatPolicy;
using auxilia::OkStatus;
using auxilia::Status;
Resolver::Resolver(loxo::interpreter &interpreter) : interpreter(interpreter) {}

auto Resolver::resolve(const expression::Expr &expr) const -> eval_result_t {
  return expr.accept(*this);
}
auto Resolver::resolve(const statement::Stmt &stmt) const -> Status {
  return stmt.accept(*this);
}

auto Resolver::resolve(
    const std::span<const std::shared_ptr<statement::Stmt>> stmts) const
    -> eval_result_t {
  for (const auto &stmt : stmts)
    if (auto res = resolve(*stmt); !res)
      return res;

  return {};
}

auto Resolver::resolve(const expression::Expr &expr, const Token &token)
    -> eval_result_t {
  for (auto it = scopes.rbegin(); it != scopes.rend(); ++it)
    if (it->contains(token.to_string(kDetailed))) {
      interpreter.resolve(expr, std::ranges::distance(scopes.rbegin(), it));
      return {};
    }
  return {};
}
auto Resolver::resolve(const statement::Function &stmt) -> eval_result_t {
  define(stmt.name);

  scope_guard guard(scopes);

  std::ranges::for_each(stmt.parameters,
                        [this](const auto &param) { define(param); });

  return resolve(stmt.body.statements);
}
void Resolver::declare(const Token &token) {
  return this->add_to_scope(token, false);
}
void Resolver::define(const Token &token) {
  return this->add_to_scope(token, true);
}

void Resolver::add_to_scope(const Token &token, const bool is_defined) {
  if (scopes.empty())
    return;
  scopes.back().insert_or_assign(token.to_string(kDetailed), is_defined);
}
auto Resolver::visit_impl(const expression::Literal &) -> eval_result_t {
  // nothing to do
  return {};
}
auto Resolver::visit_impl(const expression::Unary &expr) -> eval_result_t {
  return resolve(*expr.expr);
}
auto Resolver::visit_impl(const expression::Binary &expr) -> eval_result_t {
  return resolve(*expr.left) && resolve(*expr.right);
}
auto Resolver::visit_impl(const expression::Grouping &) -> eval_result_t {
  // nothing to do
  return {};
}
auto Resolver::visit_impl(const expression::Variable &expr) -> eval_result_t {
  if (!scopes.empty() and
      scopes.back().contains(expr.name.to_string(kDetailed)) and
      scopes.back()[expr.name.to_string(kDetailed)] == false) {
    // return {auxilia::InvalidArgumentError(
    //     "Cannot read local variable '{}' in its own initializer.\n[line {}]",
    //     expr.name.to_string(kDetailed),
    //     expr.name.line)};
    dbg(error,
        "Cannot read local variable '{}' in its own initializer.\n[line {}]",
        expr.name.to_string(kDetailed),
        expr.name.line)
    return {};
  }
  return resolve(expr, expr.name);
}
auto Resolver::visit_impl(const expression::Assignment &expr) -> eval_result_t {
  return resolve(*expr.value_expr) && resolve(expr, expr.name);
}
auto Resolver::visit_impl(const expression::Logical &expr) -> eval_result_t {
  return resolve(*expr.left) && resolve(*expr.right);
}
auto Resolver::visit_impl(const expression::Call &expr) -> eval_result_t {
  auto res = resolve(*expr.callee);
  std::ranges::for_each(
      expr.args, [this, &res](const auto &arg) { res &= resolve(*arg); });
  return res;
}
auto Resolver::evaluate_impl(const expression::Expr &expr) -> eval_result_t {
  TODO()
}
auto Resolver::get_result_impl() const -> eval_result_t { TODO() }

auto Resolver::visit_impl(const statement::Variable &stmt) -> eval_result_t {
  declare(stmt.name);

  if (stmt.has_initilizer())
    if (auto res = resolve(*stmt.initializer); !res)
      return res;

  define(stmt.name);
  return {};
}
auto Resolver::visit_impl(const statement::Print &stmt) -> eval_result_t {
  return resolve(*stmt.value);
}
auto Resolver::visit_impl(const statement::Expression &stmt) -> eval_result_t {
  return resolve(*stmt.expr);
}
auto Resolver::visit_impl(const statement::If &stmt) -> eval_result_t {
  return resolve(*stmt.condition) && resolve(*stmt.then_branch) &&
         (stmt.else_branch ? resolve(*stmt.else_branch) : OkStatus());
}
auto Resolver::visit_impl(const statement::While &stmt) -> eval_result_t {
  return resolve(*stmt.condition) && resolve(*stmt.body);
}
auto Resolver::visit_impl(const statement::For &stmt) -> eval_result_t {
  // should not exists, should be desugared in the parser.
  // nonetheless I did not desugar it.

  // {
  //   initializer;
  //   while (condition)
  //     body and increment;
  // }
  // the `body` might not be a Block itself. just recursively resolve it.

  scope_guard guard(scopes);

  return (stmt.initializer ? resolve(*stmt.initializer) : OkStatus()) &&
         resolve(*stmt.condition) &&
         (stmt.body ? resolve(*stmt.body) : OkStatus()) &&
         (stmt.increment ? resolve(*stmt.increment) : OkStatus());
}
auto Resolver::visit_impl(const statement::Function &stmt) -> eval_result_t {
  return resolve(stmt);
}
auto Resolver::visit_impl(const statement::Return &stmt) -> eval_result_t {
  return stmt.value ? resolve(*stmt.value) : OkStatus();
}
auto Resolver::execute_impl(const statement::Stmt &) -> eval_result_t { TODO() }
auto Resolver::visit_impl(const statement::Block &stmt) -> eval_result_t {
  scope_guard guard(scopes);

  return resolve(stmt.statements);
}

auto Resolver::to_string(const auxilia::FormatPolicy &) const -> string_type {
  TODO()
}
} // namespace accat::loxo
