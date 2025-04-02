#include "ASTPrinter.hpp"

#include <ostream>
#include <source_location>
#include <sstream>
#include <utility>
#include <variant>

#include <accat/auxilia/auxilia.hpp>

#include "accat/auxilia/details/format.hpp"
#include "details/loxo_fwd.hpp"
#include "details/IVisitor.hpp"

#include "expression.hpp"

namespace accat::loxo::expression {
ASTPrinter::eval_result_t ASTPrinter::visit_impl(const Grouping &expr) {
  dbg(trace, "Grouping: {}", expr.to_string(auxilia::FormatPolicy::kDefault))
  oss << expr << std::endl;
  return {};
}
ASTPrinter::eval_result_t ASTPrinter::visit_impl(const Variable &expr) {
  dbg(trace, "Variable: {}", expr.to_string(auxilia::FormatPolicy::kDefault))
  oss << expr << std::endl;
  return {};
}
IVisitor::eval_result_t ASTPrinter::visit_impl(const Assignment &expr) {
  dbg(trace, "Assignment: {}", expr.to_string(auxilia::FormatPolicy::kDefault))
  oss << expr << std::endl;
  return {};
}
ASTPrinter::eval_result_t ASTPrinter::visit_impl(const Logical &expr) {
  dbg(trace, "Logical: {}", expr.to_string(auxilia::FormatPolicy::kDefault))
  oss << expr << std::endl;
  return {};
}
ASTPrinter::eval_result_t ASTPrinter::visit_impl(const Call &expr) {
  dbg(trace, "Call: {}", expr.to_string(auxilia::FormatPolicy::kDefault))
  oss << expr << std::endl;
  return {};
}
auto ASTPrinter::evaluate_impl(const Expr &expr) -> eval_result_t {
  const_cast<eval_result_t &>(res) = expr.accept(*this);
  return {};
}
ASTPrinter::eval_result_t ASTPrinter::visit_impl(const Literal &expr) {
  dbg(trace, "Literal: {}", expr.to_string(auxilia::FormatPolicy::kDefault))
  oss << expr << std::endl;
  return {};
}
ASTPrinter::eval_result_t ASTPrinter::visit_impl(const Unary &expr) {
  dbg(trace, "Unary: {}", expr.to_string(auxilia::FormatPolicy::kDefault))
  oss << expr << std::endl;
  return {};
}
ASTPrinter::eval_result_t ASTPrinter::visit_impl(const Binary &expr) {
  dbg(trace, "Binary: {}", expr.to_string(auxilia::FormatPolicy::kDefault))
  oss << expr << std::endl;
  return {};
}
auto ASTPrinter::to_string(const auxilia::FormatPolicy &format_policy) const
    -> string_type {
  return oss.str();
}
auto ASTPrinter::to_string_view(const auxilia::FormatPolicy &) const
    -> string_view_type {
  return oss.view();
}
ExprVisitor::eval_result_t ASTPrinter::get_result_impl() const { return res; }
} // namespace accat::loxo::expression
