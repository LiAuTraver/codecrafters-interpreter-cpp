#pragma once

#include <memory>
#include <span>

#include "Environment.hpp"
#include "config.hpp"
#include "fmt.hpp"
#include "loxo_fwd.hpp"

#include "ExprVisitor.hpp"
#include "statement.hpp"
namespace net::ancillarycat::loxograph {
/// @implements expression::ExprVisitor
class LOXOGRAPH_API interpreter : virtual public expression::ExprVisitor,
                                  virtual public statement::StmtVisitor {
public:
  interpreter() = default;
  virtual ~interpreter() override = default;
  using ostringstream_t = std::ostringstream;

public:
  utils::Status interpret(std::span<std::shared_ptr<statement::Stmt>>) const;

private:
  virtual eval_result_t visit_impl(const expression::Literal &) const override;
  virtual eval_result_t visit_impl(const expression::Unary &) const override;
  virtual eval_result_t visit_impl(const expression::Binary &) const override;
  virtual eval_result_t visit_impl(const expression::Grouping &) const override;
  virtual eval_result_t visit_impl(const expression::Variable &) const override;
  virtual eval_result_t
  visit_impl(const expression::IllegalExpr &) const override;
  virtual utils::Status evaluate_impl(const expression::Expr &) const override;
  /// @note in Lisp/Scheme, only `#f` is false, everything else is true; we also
  /// make `nil` as false.
  evaluation::Boolean is_true_value(const eval_result_t &) const;
  virtual eval_result_t get_result_impl() const override;
  eval_result_t is_deep_equal(const eval_result_t &lhs,
                              const eval_result_t &) const;

private:
  virtual utils::Status visit_impl(const statement::Variable &) const override;
  virtual utils::Status visit_impl(const statement::Print &) const override;
  virtual utils::Status
  visit_impl(const statement::Expression &) const override;
  utils::Status execute_impl(const statement::Stmt &) const override;

private:
  /// @remark `mutable` wasn't intentional, but my design is flawed and this is
  /// a temporary fix.
  mutable eval_result_t expr_res{utils::Monostate{}};
  mutable std::vector<eval_result_t> stmts_res{};

private:
  auto expr_to_string(const utils::FormatPolicy &) const -> string_type;
  auto value_to_string(const utils::FormatPolicy &, const eval_result_t &) const
      -> string_type;
  virtual auto to_string_impl(const utils::FormatPolicy &) const
      -> string_type override;

private:
  friend LOXOGRAPH_API void delete_interpreter_fwd(interpreter *);

private:
  mutable evaluation::Environment env{};
};
} // namespace net::ancillarycat::loxograph
