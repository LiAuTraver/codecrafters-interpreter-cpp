#include <any>
#include <ostream>
#include <source_location>
#include <sstream>
#include <utility>
#include <variant>

#include "details/loxo_fwd.hpp"
#include "details/IVisitor.hpp"
#include "ASTPrinter.hpp"
#include "expression.hpp"
#include "net/ancillarycat/utils/Status.hpp"

namespace net::ancillarycat::loxo::expression {
ASTPrinter::eval_result_t ASTPrinter::visit_impl(const Grouping &expr) const {
  dbg(trace, "Grouping: {}", expr.to_string())
  oss << expr << std::endl;
  return {};
}
ASTPrinter::eval_result_t ASTPrinter::visit_impl(const Variable &expr) const {
  dbg(trace, "Variable: {}", expr.to_string())
  oss << expr << std::endl;
  return {};
}
utils::IVisitor::eval_result_t
ASTPrinter::visit_impl(const Assignment &expr) const {
  dbg(trace, "Assignment: {}", expr.to_string())
  oss << expr << std::endl;
  return {};
}
ASTPrinter::eval_result_t ASTPrinter::visit_impl(const Logical &expr) const {
  dbg(trace, "Logical: {}", expr.to_string())
  oss << expr << std::endl;
  return {};
}
ASTPrinter::eval_result_t ASTPrinter::visit_impl(const Call &expr) const {
  dbg(trace, "Call: {}", expr.to_string())
  oss << expr << std::endl;
  return {};
}
auto ASTPrinter::evaluate_impl(const Expr &expr) const -> eval_result_t {
  const_cast<eval_result_t &>(res) = expr.accept(*this);
  return utils::OkStatus();
}
ASTPrinter::eval_result_t ASTPrinter::visit_impl(const Literal &expr) const {
  dbg(trace, "Literal: {}", expr.to_string())
  oss << expr << std::endl;
  return {};
}
ASTPrinter::eval_result_t ASTPrinter::visit_impl(const Unary &expr) const {
  dbg(trace, "Unary: {}", expr.to_string())
  oss << expr << std::endl;
  return {};
}
ASTPrinter::eval_result_t ASTPrinter::visit_impl(const Binary &expr) const {
  dbg(trace, "Binary: {}", expr.to_string())
  oss << expr << std::endl;
  return {};
}
auto ASTPrinter::to_string_impl(const utils::FormatPolicy &format_policy) const
    -> string_type {
  return oss.str();
}
auto ASTPrinter::to_string_view_impl(const utils::FormatPolicy &) const
    -> utils::Viewable::string_view_type {
  return oss.view();
}
ExprVisitor::eval_result_t ASTPrinter::get_result_impl() const { return res; }
} // namespace net::ancillarycat::loxo::expression
