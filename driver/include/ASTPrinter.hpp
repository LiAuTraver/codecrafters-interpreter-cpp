#pragma once

#include "details/loxo_fwd.hpp"
#include "details/IVisitor.hpp"
#include "ExprVisitor.hpp"

namespace net::ancillarycat::loxo::expression {
  /// @implements ExprVisitor
class LOXO_API ASTPrinter : public ExprVisitor, public utils::Viewable {
public:
  using ostream_t = std::ostream;
  using ostringstream_t = std::ostringstream;

public:
  ASTPrinter() = default;
  virtual ~ASTPrinter() override = default;

private:
  virtual eval_result_t visit_impl(const Literal &) const override;
  virtual eval_result_t visit_impl(const Unary &) const override;
  virtual eval_result_t visit_impl(const Binary &) const override;
  virtual eval_result_t visit_impl(const Grouping &) const override;
  virtual eval_result_t visit_impl(const Variable &) const override;
  virtual eval_result_t visit_impl(const Assignment &) const override;
  virtual eval_result_t visit_impl(const Logical &) const override;
  virtual eval_result_t visit_impl(const Call &) const override;
  virtual eval_result_t visit_impl(const IllegalExpr &) const override;
  virtual stmt_result_t evaluate_impl(const Expr &) const override;
  virtual string_type
  to_string_impl(const utils::FormatPolicy &) const override;
  auto to_string_view_impl(const utils::FormatPolicy &) const
      -> utils::Viewable::string_view_type override;
  eval_result_t get_result_impl() const override;

private:
  eval_result_t res{utils::Monostate{}};
  mutable ostringstream_t oss;
  mutable ostringstream_t error_stream;
};
}
