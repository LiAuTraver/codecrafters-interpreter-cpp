#pragma once

#include <cstddef>
#include <memory>
#include <expected>
#include <unordered_map>
#include <utility>
#include <span>

#include <accat/auxilia/auxilia.hpp>
#include <vector>

#include "Token.hpp"
#include "accat/auxilia/details/format.hpp"
#include "details/loxo_fwd.hpp"

#include "expression.hpp"
#include "statement.hpp"
#include "ExprVisitor.hpp"
#include "StmtVisitor.hpp"

namespace accat::loxo {

template <typename SmartPtr>
concept SmartPtrLike = requires(SmartPtr ptr) {
  { ptr.get() } -> std::same_as<typename SmartPtr::element_type *>;
  { ptr.operator->() } -> std::same_as<typename SmartPtr::element_type *>;
  { ptr.operator*() } -> std::same_as<typename SmartPtr::element_type &>;
};
template <SmartPtrLike SmartPtr> struct Hasher {
  std::size_t operator()(const SmartPtr &ptr) const {
    return std::hash<std::uintptr_t>()(
        reinterpret_cast<std::uintptr_t>(ptr.get()));
  }
};

template <SmartPtrLike SmartPtr> struct Equal {
  bool operator()(const SmartPtr &lhs, const SmartPtr &rhs) const {
    return *lhs == *rhs;
  }
};

/// @implements expression::ExprVisitor
class LOXO_API interpreter : public auxilia::Printable,
                             virtual public expression::ExprVisitor,
                             virtual public statement::StmtVisitor,
                             std::enable_shared_from_this<interpreter> {
public:
  interpreter();
  virtual ~interpreter() override = default;
  using ostringstream_t = std::ostringstream;
  using env_t = Environment;
  using env_ptr_t = std::shared_ptr<env_t>;
  using local_env_t =
      std::unordered_map<std::shared_ptr<const expression::Expr>, size_t,
                         Hasher<std::shared_ptr<const expression::Expr>>,
                         Equal<std::shared_ptr<const expression::Expr>>>;

public:
  eval_result_t interpret(std::span<std::shared_ptr<statement::Stmt>>);
  auto set_env(const env_ptr_t &) -> interpreter &;
  auto get_current_env() { return env; }
  size_t resolve(const std::shared_ptr<const expression::Expr> &, size_t);

private:
  virtual auto visit_impl(const expression::Literal &)
      -> eval_result_t override;
  virtual auto visit_impl(const expression::Unary &) -> eval_result_t override;
  virtual auto visit_impl(const expression::Binary &) -> eval_result_t override;
  virtual auto visit_impl(const expression::Grouping &)
      -> eval_result_t override;
  virtual auto visit_impl(const expression::Variable &)
      -> eval_result_t override;
  virtual auto visit_impl(const expression::Assignment &)
      -> eval_result_t override;
  virtual auto visit_impl(const expression::Logical &)
      -> eval_result_t override;
  virtual auto visit_impl(const expression::Call &) -> eval_result_t override;
  virtual auto visit_impl(const expression::Get &) -> eval_result_t override;
  virtual auto visit_impl(const expression::Set &) -> eval_result_t override;
  virtual auto visit_impl(const expression::This &) -> eval_result_t override;

private:
  virtual auto evaluate_impl(const expression::Expr &)
      -> eval_result_t override;
  virtual auto get_result_impl() const -> eval_result_t override;

private:
  /// @note in Lisp/Scheme, only `#f` is false, everything else is true; we also
  /// make `nil` as false.
  evaluation::Boolean is_true_value(const eval_result_t &) const;
  evaluation::Boolean is_deep_equal(const eval_result_t &,
                                    const eval_result_t &) const;
  auto get_call_args(const expression::Call &) const
      -> auxilia::StatusOr<std::vector<variant_type>>;
  auto get_function(const statement::Function &, bool = false)
      -> evaluation::Function;
  auto find_variable(const expression::Expr &, const Token &) -> eval_result_t;

private:
  virtual auto visit_impl(const statement::Variable &)
      -> eval_result_t override;
  virtual auto visit_impl(const statement::Print &) -> eval_result_t override;
  virtual auto visit_impl(const statement::Expression &)
      -> eval_result_t override;
  virtual auto visit_impl(const statement::Block &) -> eval_result_t override;
  virtual auto visit_impl(const statement::If &) -> eval_result_t override;
  virtual auto visit_impl(const statement::While &) -> eval_result_t override;
  virtual auto visit_impl(const statement::For &) -> eval_result_t override;
  virtual auto visit_impl(const statement::Function &)
      -> eval_result_t override;
  virtual auto visit_impl(const statement::Class &) -> eval_result_t override;
  virtual auto visit_impl(const statement::Return &) -> eval_result_t override;
  virtual auto execute_impl(const statement::Stmt &) -> eval_result_t override;

private:
  eval_result_t last_expr_res{auxilia::Monostate{}};
  std::vector<eval_result_t> stmts_res{};
  env_ptr_t env{};
  local_env_t local_env{};
  // temporary fix, is it's true, do not `to_string` for last_expr.
  bool is_interpreting_stmts = false;

private:
  auto expr_to_string(const auxilia::FormatPolicy &) const -> string_type;
  auto value_to_string(const auxilia::FormatPolicy &,
                       const eval_result_t &) const -> string_type;

public:
  auto to_string(const auxilia::FormatPolicy & =
                     auxilia::FormatPolicy::kDefault) const -> string_type;

private:
  [[nodiscard]] inline interpreter::eval_result_t
  Returning(interpreter::eval_result_t &&res) {
    this->last_expr_res.reset(res.value()).ignore_error();
    return {auxilia::Status::kReturning, res.value()};
  }
  [[nodiscard]] inline interpreter::eval_result_t
  Returning(interpreter::eval_result_t &res) {
    this->last_expr_res.reset(res.value()).ignore_error();
    return {auxilia::Status::kReturning, res.value()};
  }

private:
  friend LOXO_API void delete_interpreter_fwd(interpreter *);
  /// @brief basic RAII guard for entering a new scope.
  struct environment_guard;
};
} // namespace accat::loxo
