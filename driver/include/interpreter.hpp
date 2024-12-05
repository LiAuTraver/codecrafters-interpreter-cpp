#pragma once

#include <memory>
#include <expected>
#include <utility>
#include <span>

#include <net/ancillarycat/utils/Status.hpp>

#include "details/loxo_fwd.hpp"


#include "expression.hpp"
#include "statement.hpp"
#include "ExprVisitor.hpp"
#include "StmtVisitor.hpp"
#include "Environment.hpp"

namespace net::ancillarycat::loxo {
/// @implements expression::ExprVisitor
class LOXO_API interpreter : virtual public expression::ExprVisitor,
                             virtual public statement::StmtVisitor {
public:
  interpreter();
  virtual ~interpreter() override = default;
  using ostringstream_t = std::ostringstream;
  using env_t = Environment;
  using env_ptr_t = std::shared_ptr<env_t>;

public:
  stmt_result_t interpret(std::span<std::shared_ptr<statement::Stmt>>) const;
  auto save_and_renew_env() const -> const interpreter &;
  auto restore_env() const -> const interpreter &;
  auto get_current_env() const -> std::weak_ptr<env_t> { return env; }

private:
  virtual auto visit_impl(const expression::Literal &) const
      -> eval_result_t override;
  virtual auto visit_impl(const expression::Unary &) const
      -> eval_result_t override;
  virtual auto visit_impl(const expression::Binary &) const
      -> eval_result_t override;
  virtual auto visit_impl(const expression::Grouping &) const
      -> eval_result_t override;
  virtual auto visit_impl(const expression::Variable &) const
      -> eval_result_t override;
  virtual auto visit_impl(const expression::Assignment &) const
      -> eval_result_t override;
  virtual auto visit_impl(const expression::Logical &) const
      -> eval_result_t override;
  virtual auto visit_impl(const expression::Call &) const
      -> eval_result_t override;
  virtual auto visit_impl(const expression::IllegalExpr &) const
      -> eval_result_t override;

private:
  virtual auto evaluate_impl(const expression::Expr &) const
      -> stmt_result_t override;
  virtual auto get_result_impl() const -> eval_result_t override;

private:
  /// @note in Lisp/Scheme, only `#f` is false, everything else is true; we also
  /// make `nil` as false.
  evaluation::Boolean is_true_value(const eval_result_t &) const;
  auto is_deep_equal(const eval_result_t &, const eval_result_t &) const
      -> eval_result_t;
  auto get_call_args(const expression::Call &expr) const
      -> std::expected<std::vector<eval_result_t>, eval_result_t>;

private:
  virtual auto visit_impl(const statement::Variable &) const
      -> stmt_result_t override;
  virtual auto visit_impl(const statement::Print &) const
      -> stmt_result_t override;
  virtual auto visit_impl(const statement::Expression &) const
      -> stmt_result_t override;
  virtual auto visit_impl(const statement::Block &) const
      -> stmt_result_t override;
  virtual auto visit_impl(const statement::If &) const
      -> stmt_result_t override;
  virtual auto visit_impl(const statement::While &) const
      -> stmt_result_t override;
  virtual auto visit_impl(const statement::For &) const
      -> stmt_result_t override;
  virtual auto visit_impl(const statement::Function &) const
      -> stmt_result_t override;
  virtual auto visit_impl(const statement::Return &) const
      -> stmt_result_t override;
  virtual auto visit_impl(const statement::IllegalStmt &) const
      -> stmt_result_t override;
  virtual auto execute_impl(const statement::Stmt &) const
      -> stmt_result_t override;

private:
  /// @remark `mutable` wasn't intentional, but my design is flawed and this is
  /// a temporary fix.
  mutable eval_result_t expr_res{utils::Monostate{}};
  mutable std::vector<eval_result_t> stmts_res{};
  mutable env_ptr_t env{};
  mutable env_ptr_t prev_env{};

private:
  auto expr_to_string(const utils::FormatPolicy &) const -> string_type;
  auto value_to_string(const utils::FormatPolicy &, const eval_result_t &) const
      -> string_type;
  virtual auto to_string_impl(const utils::FormatPolicy &) const
      -> string_type override;

private:
  friend LOXO_API void delete_interpreter_fwd(interpreter *);
};
} // namespace net::ancillarycat::loxo
