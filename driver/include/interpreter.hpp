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
#include "details/lox_fwd.hpp"

#include "expression.hpp"
#include "statement.hpp"
#include "ExprVisitor.hpp"
#include "StmtVisitor.hpp"

namespace accat::lox {

// template <typename SmartPtr>
// concept SmartPtrLike = requires(SmartPtr ptr) {
//   { ptr.get() } -> std::same_as<typename SmartPtr::element_type *>;
//   { ptr.operator->() } -> std::same_as<typename SmartPtr::element_type *>;
//   { ptr.operator*() } -> std::same_as<typename SmartPtr::element_type &>;
// };
// template <SmartPtrLike SmartPtr> struct Hasher {
//   std::size_t operator()(const SmartPtr &ptr) const {
//     return std::hash<std::uintptr_t>()(
//         reinterpret_cast<std::uintptr_t>(ptr.get()));
//   }
// };
//
// template <SmartPtrLike SmartPtr> struct Equal {
//   bool operator()(const SmartPtr &lhs, const SmartPtr &rhs) const {
//     return *lhs == *rhs;
//   }
// };

/// @implements expression::ExprVisitor
class AC_LOX_API interpreter : public auxilia::Printable,
                             virtual public expression::ExprVisitor,
                             virtual public statement::StmtVisitor,
                             std::enable_shared_from_this<interpreter> {
  /// @note previously I use std::unordered_map<cexpr_ptr_t, size_t,
  /// Hasher<...>, Equal<...>>, but it seems the `Equal` function will not be
  /// called if we call `emplace`,`contains`, and `find`.
  struct ResolvedEnv : Printable {
    using cexpr_ptr_t = std::shared_ptr<const expression::Expr>;
    using resolved_env_t = std::unordered_map<cexpr_ptr_t, size_t>;

  private:
    resolved_env_t realLocalEnv;

  public:
    auto find(this auto &&self, const cexpr_ptr_t &expr) {
      for (auto it = self.realLocalEnv.begin(); it != self.realLocalEnv.end();
           ++it) {
        if (*it->first == *expr) {
          return it;
        }
      }
      return self.realLocalEnv.end();
    }
    bool emplace(const cexpr_ptr_t &expr, const size_t &depth) {
      if (find(expr) == realLocalEnv.end())
        return realLocalEnv.emplace(expr, depth).second;
      return false;
    }
    dbg_only([[gnu::used]])
    auto to_string(const auxilia::FormatPolicy &format_policy =
                       auxilia::FormatPolicy::kDefault) const -> string_type {
      ostringstream_t oss;
      for (const auto &[expr, depth] : realLocalEnv) {
        oss << expr->to_string(format_policy) << " : " << depth << "\n";
      }
      return oss.str();
    }
    auto end(this auto &&self) { return self.realLocalEnv.end(); }
    auto contains(this auto &&self, const cexpr_ptr_t &expr) -> bool {
      return self.realLocalEnv.find(expr) != self.realLocalEnv.end();
    }
  };

public:
  interpreter();
  virtual ~interpreter() override = default;
  using ostringstream_t = std::ostringstream;
  using env_t = Environment;
  using env_ptr_t = std::shared_ptr<env_t>;
  using local_env_t = ResolvedEnv;
  using cexpr_ptr_t = local_env_t::cexpr_ptr_t;

public:
  eval_result_t interpret(std::span<std::shared_ptr<statement::Stmt>>);
  auto set_env(const env_ptr_t &) -> interpreter &;
  auto get_current_env() { return env; }
  size_t resolve(const std::shared_ptr<const expression::Expr> &, size_t);

private:
  virtual auto visit2(const expression::Literal &) -> eval_result_t override;
  virtual auto visit2(const expression::Unary &) -> eval_result_t override;
  virtual auto visit2(const expression::Binary &) -> eval_result_t override;
  virtual auto visit2(const expression::Grouping &) -> eval_result_t override;
  virtual auto visit2(const expression::Variable &) -> eval_result_t override;
  virtual auto visit2(const expression::Assignment &) -> eval_result_t override;
  virtual auto visit2(const expression::Logical &) -> eval_result_t override;
  virtual auto visit2(const expression::Call &) -> eval_result_t override;
  virtual auto visit2(const expression::Get &) -> eval_result_t override;
  virtual auto visit2(const expression::Set &) -> eval_result_t override;
  virtual auto visit2(const expression::This &) -> eval_result_t override;
  virtual auto visit2(const expression::Super &) -> eval_result_t override;

private:
  virtual auto evaluate4(const expression::Expr &) -> eval_result_t override;
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
  auto find_variable(const cexpr_ptr_t &, const Token &) -> eval_result_t;

private:
  virtual auto visit2(const statement::Variable &) -> eval_result_t override;
  virtual auto visit2(const statement::Print &) -> eval_result_t override;
  virtual auto visit2(const statement::Expression &) -> eval_result_t override;
  virtual auto visit2(const statement::Block &) -> eval_result_t override;
  virtual auto visit2(const statement::If &) -> eval_result_t override;
  virtual auto visit2(const statement::While &) -> eval_result_t override;
  virtual auto visit2(const statement::For &) -> eval_result_t override;
  virtual auto visit2(const statement::Function &) -> eval_result_t override;
  virtual auto visit2(const statement::Class &) -> eval_result_t override;
  virtual auto visit2(const statement::Return &) -> eval_result_t override;
  virtual auto execute4(const statement::Stmt &) -> eval_result_t override;

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
    this->last_expr_res.reset(*res).ignore_error();
    return {auxilia::Status::kReturning, *res};
  }
  [[nodiscard]] inline interpreter::eval_result_t
  Returning(interpreter::eval_result_t &res) {
    this->last_expr_res.reset(*res).ignore_error();
    return {auxilia::Status::kReturning, *res};
  }

private:
  friend AC_LOX_API void delete_interpreter_fwd(interpreter *);
  /// @brief basic RAII guard for entering a new scope.
  struct environment_guard;
};
} // namespace accat::lox
