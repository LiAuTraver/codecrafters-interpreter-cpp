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
using auxilia::FormatPolicy;
using enum auxilia::FormatPolicy;

auto ASTPrinter::evaluate_impl(const Expr &expr) -> eval_result_t {
  return res = expr.accept(*this);
}
auto ASTPrinter::visit_impl(const Literal &expr) -> eval_result_t {
  dbg(trace, "Literal: {}", expr.to_string(kDefault))
  oss << expr << std::endl;
  return {};
}
auto ASTPrinter::visit_impl(const Unary &expr) -> eval_result_t {
  dbg(trace, "Unary: {}", expr.to_string(kDefault))
  oss << expr << std::endl;
  return {};
}
auto ASTPrinter::visit_impl(const Binary &expr) -> eval_result_t {
  dbg(trace, "Binary: {}", expr.to_string(kDefault))
  oss << expr << std::endl;
  return {};
}
auto ASTPrinter::to_string(const FormatPolicy &format_policy) const
    -> string_type {
  return oss.str();
}
auto ASTPrinter::to_string_view(const FormatPolicy &) const
    -> string_view_type {
  return oss.view();
}
auto ASTPrinter::visit_impl(const Grouping &expr) -> eval_result_t {
  dbg(trace, "Grouping: {}", expr.to_string(kDefault))
  oss << expr << std::endl;
  return {};
}
auto ASTPrinter::visit_impl(const Variable &expr) -> eval_result_t {
  dbg(trace, "Variable: {}", expr.to_string(kDefault))
  oss << expr << std::endl;
  return {};
}
auto ASTPrinter::visit_impl(const Assignment &expr) -> eval_result_t {
  dbg(trace, "Assignment: {}", expr.to_string(kDefault))
  oss << expr << std::endl;
  return {};
}
auto ASTPrinter::visit_impl(const Logical &expr) -> eval_result_t {
  dbg(trace, "Logical: {}", expr.to_string(kDefault))
  oss << expr << std::endl;
  return {};
}
auto ASTPrinter::visit_impl(const Call &expr) -> eval_result_t {
  dbg(trace, "Call: {}", expr.to_string(kDefault))
  oss << expr << std::endl;
  return {};
}
auto ASTPrinter::visit_impl(const Get &expr) -> eval_result_t {
  dbg(trace, "Get: {}", expr.to_string(kDefault))
  oss << expr << std::endl;
  return {};
}
auto ASTPrinter::visit_impl(const Set &expr) -> eval_result_t {
  dbg(trace, "Set: {}", expr.to_string(kDefault))
  oss << expr << std::endl;
  return {};
}
auto ASTPrinter::get_result_impl() const -> eval_result_t { return res; }
} // namespace accat::loxo::expression
