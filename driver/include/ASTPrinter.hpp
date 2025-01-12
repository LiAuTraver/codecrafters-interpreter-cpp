#pragma once

#include "accat/auxilia/details/format.hpp"
#include "details/loxo_fwd.hpp"
#include "details/IVisitor.hpp"
#include "ExprVisitor.hpp"

namespace accat::loxo::expression {
/// @implements ExprVisitor
class LOXO_API ASTPrinter : public ExprVisitor,
                            public auxilia::Viewable<ASTPrinter> {
public:
  using ostream_t = std::ostream;
  using ostringstream_t = std::ostringstream;
  using string_type = auxilia::string;
  using string_view_type = auxilia::string_view;

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
  virtual eval_result_t evaluate_impl(const Expr &) const override;

public:
  string_type to_string(const auxilia::FormatPolicy & = auxilia::FormatPolicy::kDefault) const;
  auto to_string_view(const auxilia::FormatPolicy & = auxilia::FormatPolicy::kDefault) const -> string_view_type;
  eval_result_t get_result_impl() const override;

private:
  eval_result_t res{auxilia::Monostate{}};
  mutable ostringstream_t oss;
  mutable ostringstream_t error_stream;
};
} // namespace accat::loxo::expression
