#pragma once

#include <memory>
#include <span>

#include "ScopeAssoc.hpp"
#include "config.hpp"
#include "expression.hpp"
#include "status.hpp"
#include "utils.hpp"
#include "loxo_fwd.hpp"

#include "ExprVisitor.hpp"
#include "statement.hpp"
#include "Environment.hpp"

#include <expected>

namespace net::ancillarycat::loxograph {
/// @implements expression::ExprVisitor
class LOXOGRAPH_API interpreter : virtual public expression::ExprVisitor,
                                  virtual public statement::StmtVisitor {
public:
  interpreter();
  virtual ~interpreter() override = default;
  using ostringstream_t = std::ostringstream;
  using env_t = Environment;
  using env_ptr_t = std::shared_ptr<env_t>;

public:
  utils::Status interpret(std::span<std::shared_ptr<statement::Stmt>>) const;

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
  virtual auto evaluate_impl(const expression::Expr &) const
      -> utils::Status override;
  /// @note in Lisp/Scheme, only `#f` is false, everything else is true; we also
  /// make `nil` as false.
  evaluation::Boolean is_true_value(const eval_result_t &) const;
  virtual auto get_result_impl() const -> eval_result_t override;
  auto is_deep_equal(const eval_result_t &, const eval_result_t &) const
      -> eval_result_t;
  auto get_function_args(const expression::Call &expr) const
      -> std::expected<std::vector<eval_result_t>, eval_result_t>;

private:
  virtual auto visit_impl(const statement::Variable &) const
      -> utils::Status override;
  virtual auto visit_impl(const statement::Print &) const
      -> utils::Status override;
  virtual auto visit_impl(const statement::Expression &) const
      -> utils::Status override;
  virtual auto visit_impl(const statement::Block &) const
      -> utils::Status override;
  virtual auto visit_impl(const statement::If &) const
      -> utils::Status override;
  virtual auto visit_impl(const statement::While &) const
      -> utils::Status override;
  virtual auto visit_impl(const statement::For &) const
      -> utils::Status override;
  virtual auto visit_impl(const statement::Function &) const
      -> utils::Status override;
  virtual auto visit_impl(const statement::IllegalStmt &) const
      -> utils::Status override;
  virtual auto execute_impl(const statement::Stmt &) const
      -> utils::Status override;

private:
  /// @remark `mutable` wasn't intentional, but my design is flawed and this is
  /// a temporary fix.
  mutable eval_result_t expr_res{utils::Monostate{}};
  mutable std::vector<eval_result_t> stmts_res{};
  mutable env_ptr_t env{};

private:
  auto expr_to_string(const utils::FormatPolicy &) const -> string_type;
  auto value_to_string(const utils::FormatPolicy &, const eval_result_t &) const
      -> string_type;
  virtual auto to_string_impl(const utils::FormatPolicy &) const
      -> string_type override;

private:
  friend LOXOGRAPH_API void delete_interpreter_fwd(interpreter *);
};
} // namespace net::ancillarycat::loxograph
