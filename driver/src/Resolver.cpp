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
using auxilia::InvalidArgumentError;
using auxilia::OkStatus;
using auxilia::Status;
struct Resolver::scope_guard {
  using enum Resolver::ScopeType;
  inline constexpr explicit scope_guard(Resolver &resolver,
                                        ScopeType sc) noexcept
      : resolver(resolver) {
    enclosing_scope_type = resolver.current_scope_type;
    // if sc == kNone, ignore it.
    if (sc != kNone) {
      resolver.current_scope_type = sc;
    }
    resolver.scopes.emplace_back();
  }
  inline constexpr ~scope_guard() noexcept {
    resolver.scopes.pop_back();
    resolver.current_scope_type = enclosing_scope_type;
  }

private:
  Resolver &resolver;
  /// @brief represents the enclosing scope type.
  ScopeType enclosing_scope_type;
};

Resolver::Resolver(class ::accat::loxo::interpreter &interpreter)
    : interpreter(interpreter) {}

auto Resolver::resolve(
    const std::span<const std::shared_ptr<statement::Stmt>> stmts) const
    -> eval_result_t {
  for (const auto &stmt : stmts)
    if (auto res = execute(*stmt); !res)
      return res;

  return {};
}

auto Resolver::resolve_to_interp(const expression::Expr &expr,
                                 const Token &token) -> eval_result_t {
  for (auto it = scopes.rbegin(); it != scopes.rend(); ++it)
    if (it->contains(token.to_string(kDetailed))) {
      interpreter.resolve(expr, std::ranges::distance(scopes.rbegin(), it));
      return {};
    }
  return {};
}
auto Resolver::resolve(const statement::Function &stmt,
                       const ScopeType scopeType) -> eval_result_t {
  define(stmt.name);

  scope_guard guard(*this, scopeType);

  for (const auto &param : stmt.parameters) {
    if (is_defined(param))
      return {InvalidArgumentError("[line {}] Error at '{}': "
                                   "Already a variable with this name in "
                                   "this scope.",
                                   param.line,
                                   param.to_string(kDetailed))};
    define(param);
  }

  return resolve(stmt.body.statements);
}
void Resolver::declare(const Token &token) {
  return this->add_to_scope(token, false);
}
void Resolver::define(const Token &token) {
  return this->add_to_scope(token, true);
}
bool Resolver::is_defined(const Token &token) const {
  if (scopes.empty())
    return false;
  if (auto it = scopes.back().find(token.to_string(kDetailed));
      it != scopes.back().end()) {
    return it->second;
  }
  return false;
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
  return evaluate(*expr.expr);
}
auto Resolver::visit_impl(const expression::Binary &expr) -> eval_result_t {
  return evaluate(*expr.left) && evaluate(*expr.right);
}
auto Resolver::visit_impl(const expression::Grouping &) -> eval_result_t {
  // nothing to do
  return {};
}
auto Resolver::visit_impl(const expression::Variable &expr) -> eval_result_t {
  if (!scopes.empty() and
      scopes.back().contains(expr.name.to_string(kDetailed)) and
      scopes.back()[expr.name.to_string(kDetailed)] == false) {
    return {InvalidArgumentError("[line {}] Error at '{}': Can't read "
                                 "local variable in its own initializer.",
                                 expr.name.line,
                                 expr.name.to_string(kDetailed))};
  }
  return resolve_to_interp(expr, expr.name);
}
auto Resolver::visit_impl(const expression::Assignment &expr) -> eval_result_t {
  return evaluate(*expr.value_expr) && resolve_to_interp(expr, expr.name);
}
auto Resolver::visit_impl(const expression::Logical &expr) -> eval_result_t {
  return evaluate(*expr.left) && evaluate(*expr.right);
}
auto Resolver::visit_impl(const expression::Call &expr) -> eval_result_t {
  auto res = evaluate(*expr.callee);
  // clang-format off
  std::ranges::for_each(expr.args, [this, &res](const auto &arg) {
    res &= evaluate(*arg);
  });
  // clang-format on
  return res;
}
auto Resolver::visit_impl(const expression::Get &expr) -> eval_result_t {
  return evaluate(*expr.object);
}
auto Resolver::visit_impl(const expression::Set &expr) -> eval_result_t {
  return evaluate(*expr.object) && evaluate(*expr.value);
}
auto Resolver::visit_impl(const expression::This &expr) -> eval_result_t {
  return current_class_type == ClassType::kNone
             ? InvalidArgumentError(
                   "[line {}] "
                   "Error at '{}': Can't use 'this' outside of a "
                   "class.",
                   expr.name.line,
                   expr.name.to_string(kDetailed))
             : resolve_to_interp(expr, expr.name);
}
auto Resolver::evaluate_impl(const expression::Expr &expr) -> eval_result_t {
  return expr.accept(*this);
}
auto Resolver::get_result_impl() const -> eval_result_t { TODO() }

auto Resolver::visit_impl(const statement::Variable &stmt) -> eval_result_t {
  if (is_defined(stmt.name))
    return {InvalidArgumentError("[line {}] Error at '{}': "
                                 "Already a variable with this name in "
                                 "this scope.",
                                 stmt.name.line,
                                 stmt.name.to_string(kDetailed))};

  declare(stmt.name);

  if (stmt.has_initilizer())
    if (auto res = evaluate(*stmt.initializer); !res)
      return res;

  define(stmt.name);
  return {};
}
auto Resolver::visit_impl(const statement::Print &stmt) -> eval_result_t {
  return evaluate(*stmt.value);
}
auto Resolver::visit_impl(const statement::Expression &stmt) -> eval_result_t {
  return evaluate(*stmt.expr);
}
auto Resolver::visit_impl(const statement::If &stmt) -> eval_result_t {
  return evaluate(*stmt.condition) && execute(*stmt.then_branch) &&
         (stmt.else_branch ? execute(*stmt.else_branch).as_status()
                           : OkStatus());
}
auto Resolver::visit_impl(const statement::While &stmt) -> eval_result_t {
  return evaluate(*stmt.condition) && execute(*stmt.body);
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

  scope_guard guard(*this, ScopeType::kNone);

  return (stmt.initializer ? execute(*stmt.initializer).as_status()
                           : OkStatus()) &&
         evaluate(*stmt.condition) &&
         (stmt.body ? execute(*stmt.body).as_status() : OkStatus()) &&
         (stmt.increment ? evaluate(*stmt.increment).as_status() : OkStatus());
}
auto Resolver::visit_impl(const statement::Function &stmt) -> eval_result_t {
  return resolve(stmt, ScopeType::kFunction);
}
auto Resolver::visit_impl(const statement::Class &stmt) -> eval_result_t {
  define(stmt.name);

  scope_guard guard(*this, ScopeType::kNone);
  auto enclosing_class_type = this->current_class_type;
  this->current_class_type = ClassType::kClass;
  defer { this->current_class_type = enclosing_class_type; };

  // TODO: define 'super' in the class scope.
  this->scopes.back().emplace("this", true);

  auto res = OkStatus();
  std::ranges::for_each(stmt.methods, [this, &res](const auto &method) {
    res &= resolve(method, ScopeType::kMethod);
  });
  return res;
}
auto Resolver::visit_impl(const statement::Return &stmt) -> eval_result_t {
  if (this->current_scope_type == ScopeType::kNone)
    return {InvalidArgumentError("[line {}] Error at '{}': "
                                 "Can't return from top-level code.",
                                 stmt.line,
                                 "return")};
  return stmt.value ? evaluate(*stmt.value).as_status() : OkStatus();
}
auto Resolver::execute_impl(const statement::Stmt &stmt) -> eval_result_t {
  return stmt.accept(*this);
}
auto Resolver::visit_impl(const statement::Block &stmt) -> eval_result_t {
  scope_guard guard(*this, ScopeType::kNone);

  return resolve(stmt.statements);
}

auto Resolver::to_string(const auxilia::FormatPolicy &) const -> string_type {
  return "Resolver";
}
} // namespace accat::loxo
