#pragma once
#include <type_traits>
#include <variant>

#include "config.hpp"
#include "Evaluatable.hpp"
#include "loxo_fwd.hpp"
#include "status.hpp"

namespace net::ancillarycat::loxograph::expression {
/// @interface ExprVisitor
class ExprVisitor : public utils::Printable {
public:
  using value_t = std::variant<std::monostate,
                               eval::Keyword,
                               eval::Boolean,
                               eval::Nil,
                               eval::Number,
                               eval::String,
                               eval::ErrorSyntax>;
  using string_view_type = utils::Viewable::string_view_type;

public:
  virtual ~ExprVisitor() = default;
  // clang-format off
  /// @brief Visit the expression
  /// @attention tbh i don't really like the idea of making a virtual
  ///      function private, but coreguidelines says it's a good practice.
  /// @note  why make virtual function private? see <a href="https://stackoverflow.com/questions/2170688/private-virtual-method-in-c">here</a>
  // clang-format on
  template <typename DerivedExpr>
    requires std::is_base_of_v<Expr, DerivedExpr>
  auto visit(const DerivedExpr &expr) const {
    return visit_impl(expr);
  }
  auto evaluate(const Expr &expr) const { return evaluate_impl(expr); }
  auto get_result() const {
    return get_result_impl();
  }

private:
  virtual value_t visit_impl(const Literal &) const = 0;
  virtual value_t visit_impl(const Unary &) const = 0;
  virtual value_t visit_impl(const Binary &) const = 0;
  virtual value_t visit_impl(const Grouping &) const = 0;
  virtual value_t visit_impl(const IllegalExpr &) const = 0;
  virtual utils::Status evaluate_impl(const Expr &) const = 0;
  virtual value_t get_result_impl() const = 0;
};
/// @brief a dummy visitor that does nothing but test compilation
/// @implements ExprVisitor
class DummyVisitor : public ExprVisitor {
public:
  virtual value_t visit_impl(const Literal &) const override {
    return {std::monostate{}};
  }
  virtual value_t visit_impl(const Unary &) const override {
    return {std::monostate{}};
  }
  virtual value_t visit_impl(const Binary &) const override {
    return {std::monostate{}};
  }
  virtual value_t visit_impl(const Grouping &) const override {
    return {std::monostate{}};
  }
  virtual value_t visit_impl(const IllegalExpr &) const override {
    return {std::monostate{}};
  }

private:
  auto to_string_impl(const utils::FormatPolicy &format_policy) const
      -> string_type override {
    return "dummy visitor"s;
  }
  utils::Status evaluate_impl(const Expr &) const override {
    return utils::InvalidArgument("dummy visitor");
  }
  value_t get_result_impl() const override {
    return {std::monostate{}};
  }
} inline static constexpr _dummy_visitor;
/// @implements ExprVisitor
class LOXOGRAPH_API ExprEvaluator : public ExprVisitor {
public:
  ExprEvaluator() = default;
  virtual ~ExprEvaluator() override = default;

private:
  virtual value_t visit_impl(const Literal &) const override;
  virtual value_t visit_impl(const Unary &) const override;
  virtual value_t visit_impl(const Binary &) const override;
  virtual value_t visit_impl(const Grouping &) const override;
  virtual value_t visit_impl(const IllegalExpr &) const override;
  virtual utils::Status evaluate_impl(const Expr &) const override;
  /// @note in Lisp/Scheme, only `#f` is false, everything else is true; we also
  /// make `nil` as false.
  eval::Boolean is_true_value(const value_t &) const;
  value_t is_deep_equal(const value_t &lhs, const value_t &) const;
  virtual value_t get_result_impl() const override;
  auto to_string_impl(const utils::FormatPolicy &) const
      -> string_type override;

private:
  const value_t res{std::monostate{}};
};
/// @implements ExprVisitor
class LOXOGRAPH_API ASTPrinter : public ExprVisitor, public utils::Viewable {
public:
  using ostream_t = std::ostream;
  using ostringstream_t = std::ostringstream;

public:
  ASTPrinter() = default;
  virtual ~ASTPrinter() override = default;

private:
  virtual value_t visit_impl(const Literal &) const override;
  virtual value_t visit_impl(const Unary &) const override;
  virtual value_t visit_impl(const Binary &) const override;
  virtual value_t visit_impl(const Grouping &) const override;
  virtual value_t visit_impl(const IllegalExpr &) const override;
  virtual utils::Status evaluate_impl(const Expr &) const override;
  virtual string_type
  to_string_impl(const utils::FormatPolicy &) const override;
  auto to_string_view_impl(const utils::FormatPolicy &) const
      -> utils::Viewable::string_view_type override;
  value_t get_result_impl() const override;

private:
  value_t res{std::monostate{}};
  mutable ostringstream_t oss;
  mutable ostringstream_t error_stream;
};
} // namespace net::ancillarycat::loxograph::expression
