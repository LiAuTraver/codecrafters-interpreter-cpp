#pragma once

#include "accat/auxilia/details/format.hpp"
#include "details/loxo_fwd.hpp"
#include "details/IVisitor.hpp"
#include "ExprVisitor.hpp"

namespace accat::loxo::expression {
/// @implements ExprVisitor
class LOXO_API ASTPrinter : public ExprVisitor, public auxilia::Viewable {
public:
  using ostream_t = std::ostream;
  using ostringstream_t = std::ostringstream;
  using string_type = auxilia::string;
  using string_view_type = auxilia::string_view;

public:
  ASTPrinter() = default;
  virtual ~ASTPrinter() override = default;

private:
  virtual eval_result_t visit_impl(const Literal &) override;
  virtual eval_result_t visit_impl(const Unary &) override;
  virtual eval_result_t visit_impl(const Binary &) override;
  virtual eval_result_t visit_impl(const Grouping &) override;
  virtual eval_result_t visit_impl(const Variable &) override;
  virtual eval_result_t visit_impl(const Assignment &) override;
  virtual eval_result_t visit_impl(const Logical &) override;
  virtual eval_result_t visit_impl(const Call &) override;
  virtual eval_result_t evaluate_impl(const Expr &) override;

public:
  string_type to_string(
      const auxilia::FormatPolicy & = auxilia::FormatPolicy::kDefault) const;
  auto
  to_string_view(const auxilia::FormatPolicy & =
                     auxilia::FormatPolicy::kDefault) const -> string_view_type;
  eval_result_t get_result_impl() const override;

private:
  eval_result_t res{auxilia::Monostate{}};
  mutable ostringstream_t oss;
  mutable ostringstream_t error_stream;
};
} // namespace accat::loxo::expression
