#pragma once

#include <memory>
#include <stack>

#include "details/lox_fwd.hpp"

#include "details/IVisitor.hpp"
#include "ExprVisitor.hpp"
#include "StmtVisitor.hpp"

namespace accat::lox {
class AC_LOX_API Resolver : auxilia::Printable,
                          virtual public expression::ExprVisitor,
                          virtual public statement::StmtVisitor,
                          public std::enable_shared_from_this<Resolver> {
public:
  explicit Resolver(class ::accat::lox::interpreter &interpreter);
  virtual ~Resolver() override = default;
  using scope_t = std::unordered_map<std::string, bool>;
  using scopes_t = std::vector<scope_t>;

private:
  enum class ScopeType : std::uint8_t {
    kNone = 0,    // an ordinary scope(doesn't change the current scope type)
    kFunction,    // a function scope
    kMethod,      // a class method scope
    kInitializer, // a class initializer scope
  };
  enum class ClassType : std::uint8_t {
    kNone = 0,
    kClass,
    kDerivedClass,
  };

private:
  class ::accat::lox::interpreter &interpreter;
  scopes_t scopes;
  ScopeType current_scope_type = ScopeType::kNone;
  ClassType current_class_type = ClassType::kNone;

public:
  auto resolve(std::span<const std::shared_ptr<statement::Stmt>>) const
      -> eval_result_t;

private:
  auto resolve(const statement::Function &, ScopeType) -> eval_result_t;
  auto resolve_to_interp(const std::shared_ptr<const expression::Expr> &,
                         const Token &) -> eval_result_t;
  void declare(const Token &);
  void define(const Token &);
  bool is_defined(const Token &) const;
  void add_to_scope(const Token &, bool);

private:
  auto visit2(const expression::Literal &) -> eval_result_t override;
  auto visit2(const expression::Unary &) -> eval_result_t override;
  auto visit2(const expression::Binary &) -> eval_result_t override;
  auto visit2(const expression::Grouping &) -> eval_result_t override;
  auto visit2(const expression::Variable &) -> eval_result_t override;
  auto visit2(const expression::Assignment &) -> eval_result_t override;
  auto visit2(const expression::Logical &) -> eval_result_t override;
  auto visit2(const expression::Call &) -> eval_result_t override;
  auto visit2(const expression::Get &) -> eval_result_t override;
  auto visit2(const expression::Set &) -> eval_result_t override;
  auto visit2(const expression::This &) -> eval_result_t override;
  auto visit2(const expression::Super &) -> eval_result_t override;
  auto evaluate4(const expression::Expr &) -> eval_result_t override;
  auto get_result_impl() const -> eval_result_t override;

private:
  auto visit2(const statement::Variable &) -> eval_result_t override;
  auto visit2(const statement::Print &) -> eval_result_t override;
  auto visit2(const statement::Expression &) -> eval_result_t override;
  auto visit2(const statement::Block &) -> eval_result_t override;
  auto visit2(const statement::If &) -> eval_result_t override;
  auto visit2(const statement::While &) -> eval_result_t override;
  auto visit2(const statement::For &) -> eval_result_t override;
  auto visit2(const statement::Function &) -> eval_result_t override;
  auto visit2(const statement::Class &) -> eval_result_t override;
  auto visit2(const statement::Return &) -> eval_result_t override;
  auto execute4(const statement::Stmt &) -> eval_result_t override;

public:
  auto to_string(const auxilia::FormatPolicy & =
                     auxilia::FormatPolicy::kDefault) const -> string_type;

private:
  /// @brief basic RAII for scope management
  struct scope_guard;
};

} // namespace accat::lox
