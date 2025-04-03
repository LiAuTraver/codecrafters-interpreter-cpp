#pragma once

#include <stack>

#include "details/loxo_fwd.hpp"

#include "details/IVisitor.hpp"
#include "ExprVisitor.hpp"
#include "StmtVisitor.hpp"

namespace accat::loxo {
class LOXO_API Resolver : auxilia::Printable,
                          virtual public expression::ExprVisitor,
                          virtual public statement::StmtVisitor,
                          public std::enable_shared_from_this<Resolver> {
public:
  explicit Resolver(class ::accat::loxo::interpreter &interpreter);
  virtual ~Resolver() override = default;
  using scope_t = std::unordered_map<std::string, bool>;
  using scopes_t = std::vector<scope_t>;

private:
  enum class ScopeType : std::uint8_t {
    kNone = 0, // kNone
    kFunction,
    kClass,
  };

private:
  class ::accat::loxo::interpreter &interpreter;
  scopes_t scopes;
  ScopeType current_scope = ScopeType::kNone;

public:
  auto resolve(std::span<const std::shared_ptr<statement::Stmt>>) const
      -> eval_result_t;

private:
  auto resolve(const statement::Function &, ScopeType) -> eval_result_t;
  auto resolve_to_interp(const expression::Expr &, const Token &)
      -> eval_result_t;
  void declare(const Token &);
  void define(const Token &);
  bool is_defined(const Token &) const;
  void add_to_scope(const Token &, bool);

private:
  auto visit_impl(const expression::Literal &) -> eval_result_t override;
  auto visit_impl(const expression::Unary &) -> eval_result_t override;
  auto visit_impl(const expression::Binary &) -> eval_result_t override;
  auto visit_impl(const expression::Grouping &) -> eval_result_t override;
  auto visit_impl(const expression::Variable &) -> eval_result_t override;
  auto visit_impl(const expression::Assignment &) -> eval_result_t override;
  auto visit_impl(const expression::Logical &) -> eval_result_t override;
  auto visit_impl(const expression::Call &) -> eval_result_t override;
  auto evaluate_impl(const expression::Expr &) -> eval_result_t override;
  auto get_result_impl() const -> eval_result_t override;

private:
  auto visit_impl(const statement::Variable &) -> eval_result_t override;
  auto visit_impl(const statement::Print &) -> eval_result_t override;
  auto visit_impl(const statement::Expression &) -> eval_result_t override;
  auto visit_impl(const statement::Block &) -> eval_result_t override;
  auto visit_impl(const statement::If &) -> eval_result_t override;
  auto visit_impl(const statement::While &) -> eval_result_t override;
  auto visit_impl(const statement::For &) -> eval_result_t override;
  auto visit_impl(const statement::Function &) -> eval_result_t override;
  auto visit_impl(const statement::Return &) -> eval_result_t override;
  auto execute_impl(const statement::Stmt &) -> eval_result_t override;

public:
  auto to_string(const auxilia::FormatPolicy & =
                     auxilia::FormatPolicy::kDefault) const -> string_type;

private:
  /// @brief basic RAII for scope management
  struct scope_guard;
};

} // namespace accat::loxo
