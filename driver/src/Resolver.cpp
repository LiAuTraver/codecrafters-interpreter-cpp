#include "Resolver.hpp"

#include <algorithm>
#include <iterator>
#include <memory>
#include <string>
#include <utility>

#include "Token.hpp"
#include "accat/auxilia/details/Status.hpp"
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

auto Resolver::resolve_to_interp(
    const std::shared_ptr<const expression::Expr> &expr, const Token &token)
    -> eval_result_t {
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
auto Resolver::visit2(const expression::Literal &) -> eval_result_t {
  // nothing to do
  return {};
}
auto Resolver::visit2(const expression::Unary &expr) -> eval_result_t {
  return evaluate(*expr.expr);
}
auto Resolver::visit2(const expression::Binary &expr) -> eval_result_t {
  return evaluate(*expr.left) && evaluate(*expr.right);
}
auto Resolver::visit2(const expression::Grouping &) -> eval_result_t {
  // nothing to do
  return {};
}
auto Resolver::visit2(const expression::Variable &expr) -> eval_result_t {
  if (!scopes.empty() and
      scopes.back().contains(expr.name.to_string(kDetailed)) and
      scopes.back()[expr.name.to_string(kDetailed)] == false) {
    return {InvalidArgumentError("[line {}] Error at '{}': Can't read "
                                 "local variable in its own initializer.",
                                 expr.name.line,
                                 expr.name.to_string(kDetailed))};
  }
  return resolve_to_interp(expr.shared_from_this(), expr.name);
}
auto Resolver::visit2(const expression::Assignment &expr) -> eval_result_t {
  return evaluate(*expr.value_expr) &&
         resolve_to_interp(expr.shared_from_this(), expr.name);
}
auto Resolver::visit2(const expression::Logical &expr) -> eval_result_t {
  return evaluate(*expr.left) && evaluate(*expr.right);
}
auto Resolver::visit2(const expression::Call &expr) -> eval_result_t {
  auto res = evaluate(*expr.callee);
  // clang-format off
  std::ranges::for_each(expr.args, [this, &res](const auto &arg) {
    res &= evaluate(*arg);
  });
  // clang-format on
  return res;
}
auto Resolver::visit2(const expression::Get &expr) -> eval_result_t {
  return evaluate(*expr.object);
}
auto Resolver::visit2(const expression::Set &expr) -> eval_result_t {
  return evaluate(*expr.object) && evaluate(*expr.value);
}
auto Resolver::visit2(const expression::This &expr) -> eval_result_t {
  return current_class_type == ClassType::kNone
             ? InvalidArgumentError(
                   "[line {}] "
                   "Error at '{}': Can't use 'this' outside of a "
                   "class.",
                   expr.name.line,
                   expr.name.to_string(kDetailed))
             : resolve_to_interp(expr.shared_from_this(), expr.name);
}
auto Resolver::visit2(const expression::Super &expr) -> eval_result_t {
  if (current_class_type == ClassType::kNone)
    return {InvalidArgumentError("[line {}] Error at '{}':"
                                 "Can't use 'super' outside of a class.")};
  if (current_class_type == ClassType::kClass)
    return {InvalidArgumentError(
        "[line {}] Error at '{}': "
        "Can't use 'super' in a class with no super class.")};
  // current class type is kDerivedClass
  return resolve_to_interp(expr.shared_from_this(), expr.name);
}
auto Resolver::evaluate4(const expression::Expr &expr) -> eval_result_t {
  return expr.accept(*this);
}
auto Resolver::get_result_impl() const -> eval_result_t { TODO() }

auto Resolver::visit2(const statement::Variable &stmt) -> eval_result_t {
  if (is_defined(stmt.name))
    return {InvalidArgumentError("[line {}] Error at '{}': "
                                 "Already a variable with this name in "
                                 "this scope.",
                                 stmt.name.line,
                                 stmt.name.to_string(kDetailed))};

  declare(stmt.name);

  if (stmt.has_initializer())
    if (auto res = evaluate(*stmt.initializer); !res)
      return res;

  define(stmt.name);
  return {};
}
auto Resolver::visit2(const statement::Print &stmt) -> eval_result_t {
  return evaluate(*stmt.value);
}
auto Resolver::visit2(const statement::Expression &stmt) -> eval_result_t {
  return evaluate(*stmt.expr);
}
auto Resolver::visit2(const statement::If &stmt) -> eval_result_t {
  return evaluate(*stmt.condition) && execute(*stmt.then_branch) &&
         (stmt.else_branch ? execute(*stmt.else_branch).as_status()
                           : OkStatus());
}
auto Resolver::visit2(const statement::While &stmt) -> eval_result_t {
  return evaluate(*stmt.condition) && execute(*stmt.body);
}
auto Resolver::visit2(const statement::For &stmt) -> eval_result_t {
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
auto Resolver::visit2(const statement::Function &stmt) -> eval_result_t {
  return resolve(stmt, ScopeType::kFunction);
}
auto Resolver::visit2(const statement::Class &stmt) -> eval_result_t {
  define(stmt.name);

  auto enclosing_class_type = this->current_class_type;
  this->current_class_type = ClassType::kClass;
  defer { this->current_class_type = enclosing_class_type; };

  const bool hasSuperclass = stmt.superclass.is_type(TokenType::kIdentifier);
  if (hasSuperclass) {
    if (stmt.name == stmt.superclass)
      return {InvalidArgumentError("[line {}] Error at '{}': "
                                   "A class can't inherit from itself.",
                                   stmt.superclass.line,
                                   stmt.superclass.to_string(kDetailed))};

    this->current_class_type = ClassType::kDerivedClass;
    // design flaw
    if (auto res =
            visit2(*std::make_shared<expression::Variable>(stmt.superclass));
        !res) {
      return res;
    }
    this->scopes.emplace_back().emplace("super", true);
  }
  
  scope_guard guard(*this, ScopeType::kNone);
  this->scopes.back().emplace("this", true);

  for (const auto &method : stmt.methods)
    if (auto res = resolve(method,
                           method.name.to_string(kDetailed) == "init"
                               ? ScopeType::kInitializer
                               : ScopeType::kMethod);
        !res)
      // TODO: restore scope if the class has superclass.?(see below)
      return res;
  if (hasSuperclass)
    this->scopes.pop_back(); // pop the super class scope.
  return {};
}
auto Resolver::visit2(const statement::Return &stmt) -> eval_result_t {
  if (this->current_scope_type == ScopeType::kNone)
    return {InvalidArgumentError("[line {}] Error at '{}': "
                                 "Can't return from top-level code.",
                                 stmt.line,
                                 "return")};
  // allow to return a nil.
  if (this->current_scope_type == ScopeType::kInitializer && stmt.value)
    return {InvalidArgumentError("[line {}] Error at '{}': "
                                 "Can't return a value from an initializer.",
                                 stmt.line,
                                 "return")};
  return stmt.value ? evaluate(*stmt.value).as_status() : OkStatus();
}
auto Resolver::execute4(const statement::Stmt &stmt) -> eval_result_t {
  return stmt.accept(*this);
}
auto Resolver::visit2(const statement::Block &stmt) -> eval_result_t {
  scope_guard guard(*this, ScopeType::kNone);

  return resolve(stmt.statements);
}

auto Resolver::to_string(const auxilia::FormatPolicy &) const -> string_type {
  return "Resolver";
}
} // namespace accat::loxo
