#include <algorithm>
#include <concepts>
#include <expected>
#include <iterator>
#include <map>
#include <ranges>
#include <type_traits>
#include <utility>
#include <memory>
#include <typeinfo>
#include <vector>

#include <accat/auxilia/auxilia.hpp>

#include "Token.hpp"
#include "accat/auxilia/details/Status.hpp"
#include "accat/auxilia/details/format.hpp"
#include "accat/auxilia/details/macros.hpp"
#include "details/lox_fwd.hpp"
#include "Environment.hpp"
#include "Evaluatable.hpp"
#include "statement.hpp"
#include "expression.hpp"
#include "interpreter.hpp"

namespace accat::lox {
using auxilia::match;
using enum TokenType::type_t;
using enum auxilia::FormatPolicy;

#pragma region env
struct interpreter::environment_guard {
  class interpreter &interpreter;
  env_ptr_t original_env;
  inline explicit environment_guard(class interpreter &interpreter) noexcept
      : interpreter(interpreter), original_env(interpreter.env) {
    interpreter.env = Environment::Scope(original_env);
  }
  inline ~environment_guard() noexcept { interpreter.env = original_env; }
};

interpreter::interpreter() : env(std::make_shared<Environment>()) {}
auto interpreter::interpret(
    const std::span<std::shared_ptr<statement::Stmt>> stmts) -> eval_result_t {
  is_interpreting_stmts = true;
  this->env = Environment::Global();

  for (const auto &stmt : stmts)
    if (auto eval_res = execute(*stmt); !eval_res) {
      last_expr_res->clear();
      return eval_res;
    }

  return {};
}
size_t interpreter::resolve(const std::shared_ptr<const expression::Expr> &expr,
                            const size_t depth) {
  return local_env.emplace(expr, depth);
}

auto interpreter::set_env(const env_ptr_t &new_env) -> interpreter & {
  env = new_env;
  return *this;
}
#pragma endregion env
#pragma region statement
auto interpreter::visit2(const statement::Variable &stmt) -> eval_result_t {

  if (stmt.has_initializer()) {
    auto eval_res = evaluate(*stmt.initializer);
    if (!eval_res)
      return eval_res;
    contract_assert(stmt.name.literal.is_type<string_view_type>(),
                    "variable name should be a string")
    dbg(trace,
        "variable name: {}, value: {}",
        stmt.name.literal.get<string_view_type>(),
        eval_res->to_string())
    // string view failed again; not null-terminated
    return {
        env->add(stmt.name.to_string(kDetailed), *eval_res, stmt.name.line)};
  }
  // if no initializer, it's a nil value.
  return env->add(
      stmt.name.to_string(kDetailed), evaluation::NilValue, stmt.name.line);
}
auto interpreter::visit2(const statement::Print &stmt) -> eval_result_t {
  auto eval_res = evaluate(*stmt.value);
  if (!eval_res)
    return eval_res;
  stmts_res.emplace_back(*eval_res);
  return {{evaluation::NilValue}};
}
auto interpreter::visit2(const statement::If &stmt) -> eval_result_t {
  auto eval_res = evaluate(*stmt.condition);
  if (!eval_res)
    return eval_res;
  if (is_true_value(*eval_res).is_true()) {
    return execute(*stmt.then_branch);
  }
  // else we execute the else branch.
  if (stmt.else_branch) { // maybe we dont have an else branch, so check it.
    return execute(*stmt.else_branch);
  }
  return {*eval_res};
}
auto interpreter::visit2(const statement::While &stmt) -> eval_result_t {
  eval_result_t res;
  do {
    auto eval_res = evaluate(*stmt.condition);
    if (!eval_res)
      return eval_res;
    if (not is_true_value(*eval_res).is_true())
      break;
    res = execute(*stmt.body);
    if (!res)
      return res;
  } while (true);

  return {*res};
}
auto interpreter::visit2(const statement::For &stmt) -> eval_result_t {

  environment_guard guard(*this);

  if (stmt.initializer)
    if (auto res = execute(*stmt.initializer); !res)
      return res;

  while (true) {
    if (stmt.condition) {
      auto cond_res = evaluate(*stmt.condition);
      if (!cond_res)
        return cond_res;
      if (not is_true_value(*cond_res).is_true())
        break;
    }
    if (auto res = execute(*stmt.body); !res)
      return res;
    if (stmt.increment) {
      if (auto res = evaluate(*stmt.increment); !res)
        return res;
    }
  }
  return {};
}
auto interpreter::visit2(const statement::Function &stmt) -> eval_result_t {
  // TODO: function overloading
  if (auto res = env->get(stmt.name.to_string(kDetailed));
      res && !res->empty()) {
    if (!res->is_type<evaluation::Function>()) {
      dbg(error,
          "bad function definition: {} is not a function",
          stmt.name.to_string(kDefault))
      return *res;
    }
    // if arity is same, warn and overwrite the function;
    // if arity is different, just as overloading.
    auto callable = res->get<evaluation::Function>();
    if (callable.arity() == stmt.parameters.size()) {
      dbg(warn, "function {} already defined", stmt.name.to_string(kDetailed))
    } else {
      dbg(info, "overloading function: {}", stmt.name.to_string(kDetailed))
      TODO(unordered map, overloading)
    }
  }

  dbg(trace, "func name: {}", stmt.name.to_string(kDetailed))

  return env->add(
      stmt.name.to_string(kDetailed), get_function(stmt), stmt.name.line);
}
auto interpreter::visit2(const statement::Class &stmt) -> eval_result_t {
  if (auto res = env->find(stmt.name.to_string(kDetailed)); res) {
    TODO(...)
  }
  env_ptr_t supEnv;
  if (stmt.superclass) {
    // design flaw
    auto res = visit2(*stmt.superclass);
    if (!res)
      return res;
    if (!res->is_type<evaluation::Class>())
      return auxilia::InvalidArgumentError(
          "Superclass must be a class.\n[line {}]", stmt.superclass->name.line);

    /// cannot use environment_guard here -- scope issue
    env = Environment::Scope(env);
    env->add("super", *res, stmt.superclass->name.line);
    supEnv = env;
  }

  evaluation::Class::methods_t methods;
  std::ranges::for_each(stmt.methods, [this, &methods](auto &&method) {
    auto name = method.name.to_string(kDetailed);
    methods.emplace(name, get_function(method, name == "init" ? true : false));
  });

  if (stmt.superclass)
    env = env->ancestor(1);

  return env->add(
      stmt.name.to_string(kDetailed),
      evaluation::Class{stmt.name.to_string(kDetailed),
                        stmt.name.line,
                        std::move(methods),
                        stmt.superclass
                            ? stmt.superclass->name.to_string(kDetailed)
                            : std::string{},
                        std::move(supEnv)},
      stmt.name.line);
}
auto interpreter::visit2(const statement::Expression &stmt) -> eval_result_t {
  return evaluate(*stmt.expr);
}
auto interpreter::visit2(const statement::Block &stmt) -> eval_result_t {

  environment_guard guard(*this);

  for (const auto &scoped_stmt : stmt.statements)
    if (auto eval_res = execute(*scoped_stmt); !eval_res)
      return eval_res;

  return {};
}
auto interpreter::execute4(const statement::Stmt &stmt) -> eval_result_t {
  return stmt.accept(*this);
}
#pragma endregion statement
#pragma region expression
auto interpreter::get_result_impl() const -> eval_result_t {
  return last_expr_res;
}

auto interpreter::evaluate4(const expression::Expr &expr) -> eval_result_t {
  auto res = expr.accept(*this);
  if (!res)
    return res;
  dbg(info,
      "result: {}",
      res->is_type<auxilia::Monostate>() ? "<nothing>" : res->to_string())
  return last_expr_res.reset(*std::move(res));
}
auto interpreter::visit2(const statement::Return &expr) -> eval_result_t {
  if (this->env == Environment::Global()) {
    return {
        auxilia::InvalidArgumentError("Cannot return from top-level code.")};
  }

  if (not expr.value) {
    dbg(info, "returning nil")
    return Returning({{evaluation::NilValue}});
  }
  auto res = evaluate(*expr.value);
  dbg(info, "return value: {}", res->to_string())
  if (!res) {
    return res;
  }
  dbg(trace, "result: {}", res->to_string())
  return Returning(*res);
}
auto interpreter::visit2(const expression::Literal &expr) -> eval_result_t {
  dbg(trace, "literal type: {}", expr.literal.type)

  auto T = [&expr](const TokenType::type_t t) constexpr noexcept -> bool {
    return expr.literal.is_type(t);
  };

  if (T(kMonostate)) {
    dbg_break
  }
  if (T(kNil)) {
    return {evaluation::Nil{expr.literal.line}};
  }
  if (T(kTrue)) {
    return {evaluation::Boolean{true, expr.literal.line}};
  }
  if (T(kFalse)) {
    return {evaluation::Boolean{false, expr.literal.line}};
  }
  if (T(kString)) {
    return {evaluation::String{expr.literal.literal.get<string_view_type>(),
                               expr.literal.line}};
  }
  if (T(kNumber)) {
    return {evaluation::Number{expr.literal.literal.get<long double>(),
                               expr.literal.line}};
  }
  return {auxilia::InvalidArgumentError("Expected literal value.\n[line {}]",
                                        expr.literal.line)};
}

auto interpreter::visit2(const expression::Unary &expr) -> eval_result_t {
  auto inner_expr = expr.expr->accept(*this);
  if (expr.op.is_type(kMinus)) {
    if (inner_expr->is_type<evaluation::Number>()) {
      auto value = inner_expr->get<evaluation::Number>();
      dbg(trace, "unary minus: {}", value)
      return {evaluation::Number{value * (-1)}};
    }
    return {auxilia::InvalidArgumentError(
        "Operand must be a number.\n[line {}]", expr.op.line)};
  }
  if (expr.op.is_type(kBang)) {
    auto value = is_true_value(*inner_expr);
    dbg(trace, "unary bang: {}", value.to_string(kDefault))
    return {evaluation::Boolean{!value}};
  }
  contract_assert(false, "unreachable code reached")
  return {};
}

auto interpreter::visit2(const expression::Binary &expr) -> eval_result_t {
  auto lhs = expr.left->accept(*this);
  if (!lhs) {
    return lhs;
  }

  auto rhs = expr.right->accept(*this);
  if (!rhs) {
    return rhs;
  }

  if (expr.op.is_type(kEqualEqual)) {
    return {{is_deep_equal(lhs, rhs)}};
  }
  if (expr.op.is_type(kBangEqual)) {
    return {{!is_deep_equal(lhs, rhs)}};
  }

  if (lhs->index() != rhs->index()) {
    dbg(error, "type mismatch: lhs: {}, rhs: {}", lhs->index(), rhs->index())
    dbg(warn, "current implementation only support same type binary operation")
    return {auxilia::InvalidArgumentError(
        "Operands must be two numbers or two strings.\n[line "
        "{}]",
        expr.op.line)};
  }
  if (lhs->is_type<evaluation::String>()) {
    if (expr.op.is_type(kPlus)) {
      auto str_lhs = lhs->get<evaluation::String>();
      auto str_rhs = rhs->get<evaluation::String>();
      return {evaluation::String{str_lhs + str_rhs}};
    }
  }
  if (lhs->is_type<evaluation::Number>()) {
    auto real_lhs = lhs->get<evaluation::Number>();
    auto real_rhs = rhs->get<evaluation::Number>();
    switch (expr.op.type.type) {
    case kMinus:
      return {{real_lhs - real_rhs}};
    case kPlus:
      return {{real_lhs + real_rhs}};
    case kSlash:
      return {{real_lhs / real_rhs}};
    case kStar:
      return {{real_lhs * real_rhs}};
    case kGreater:
      return {{real_lhs > real_rhs}};
    case kGreaterEqual:
      return {{real_lhs >= real_rhs}};
    case kLess:
      return {{real_lhs < real_rhs}};
    case kLessEqual:
      return {{real_lhs <= real_rhs}};
    default:
      break;
    }
  }
  if (lhs->is_type<evaluation::Boolean>()) {
    auto bool_lhs = lhs->get<evaluation::Boolean>();
    auto bool_rhs = rhs->get<evaluation::Boolean>();
    switch (expr.op.type.type) {
    // case kGreater:
    //   return {{bool_lhs > bool_rhs}};
    // case kGreaterEqual:
    //   return {{bool_lhs >= bool_rhs}};
    // case kLess:
    //   return {{bool_lhs < bool_rhs}};
    // case kLessEqual:
    //   return {{bool_lhs <= bool_rhs}};
    case kEqualEqual:
      return {{bool_lhs == bool_rhs}};
    case kBangEqual:
      return {{bool_lhs != bool_rhs}};
    default:
      // forbids relational operators on boolean values except equality.
      return {auxilia::InvalidArgumentError(
          "Operands must be two numbers or two strings.\n[line "
          "{}]",
          expr.op.line)};
    }
  }
  dbg(critical, "unimplemented binary operator: {}", expr.op.to_string())
  return {auxilia::InvalidArgumentError(
      "unimplemented binary operator.\n[line {}]", expr.op.line)};
}
auto interpreter::visit2(const expression::Grouping &expr) -> eval_result_t {
  return {expr.expr->accept(*this)};
}
auto interpreter::visit2(const expression::Variable &expr) -> eval_result_t {
  return find_variable(expr.shared_from_this(), expr.name);
}
auto interpreter::visit2(const expression::Assignment &expr) -> eval_result_t {
  auto res = this->evaluate(*expr.value_expr);
  if (!res)
    return res;

  if (auto it = local_env.find(expr.shared_from_this());
      it != local_env.end()) {
    if (auto reassign_res = env->reassign_at_depth(
            it->second, expr.name.to_string(kDetailed), *res, expr.name.line);
        !reassign_res)
      return reassign_res;

  } else if (!Environment::Global()->reassign(
                 expr.name.to_string(kDetailed), *res, expr.name.line)) {
    return {auxilia::NotFoundError("Undefined variable '{}'.\n[line {}]",
                                   expr.name.to_string(kDetailed),
                                   expr.name.line)};
  }
  // Assignment expression returns the value of the assignment.
  return *res;
}
auto interpreter::visit2(const expression::Logical &expr) -> eval_result_t {
  auto lhs = expr.left->accept(*this);
  if (!lhs)
    return lhs;
  if (is_true_value(*lhs).is_true()) {
    if (expr.op.is_type(kOr))
      return {*lhs};
    if (expr.op.is_type(kAnd))
      return {expr.right->accept(*this)};
    contract_assert(false, "unimplemented logical operator")
    return {auxilia::Monostate{}};
  }
  // left is false, evaluate right.
  if (expr.op.is_type(kOr))
    return {expr.right->accept(*this)};
  if (expr.op.is_type(kAnd))
    return {evaluation::Boolean{false, expr.op.line}};
  contract_assert(false, "unimplemented logical operator")
  return {auxilia::Monostate{}};
}
auto interpreter::visit2(const expression::Call &expr) -> eval_result_t {
  auto res = evaluate(*expr.callee);
  if (!res)
    return res;

  // `result` would change in `get_call_args`, so we need to save it.
  const auto callee = expr.callee;
  evaluation::Callable *callable;
  // a bit less-readable, may change to a more readable version later.
  if (!((callable = res->get_if<evaluation::Function>())))
    if (!((callable = res->get_if<evaluation::Class>())))
      return {auxilia::InvalidArgumentError(
          "Can only call functions and classes.\n[line {}]", expr.paren.line)};

  auto maybe_args = get_call_args(expr);
  if (!maybe_args)
    return {maybe_args.as_status()};

  auto args = *std::move(maybe_args);
  if (args.size() == callable->arity())
    // clear `Returning` status has already been implemented in `call` method.
    // just return here.
    return callable->call(*this, std::move(args));

  return {auxilia::InvalidArgumentError(
      "Too {} arguments to call function '{}': "
      "expected {} but got {}",
      args.size() > callable->arity() ? "many" : "few",
      callee->to_string(kDefault),
      callable->arity(),
      args.size())};
}

auto interpreter::visit2(const expression::Get &expr) -> eval_result_t {
  auto res = evaluate(*expr.object);
  if (!res)
    return res;

  if (!res->is_type<evaluation::Instance>()) {
    return {auxilia::InvalidArgumentError(
        "Only instances have fields.\n[line {}]", expr.field.line)};
  }
  return {res->get<evaluation::Instance>().get_field(
      expr.field.to_string(kDetailed))};
}
auto interpreter::visit2(const expression::Set &expr) -> eval_result_t {
  auto res = evaluate(*expr.object);
  if (!res)
    return res;

  if (!res->is_type<evaluation::Instance>()) {
    return {auxilia::InvalidArgumentError(
        "Only instances have properties.\n[line {}]", expr.field.line)};
  }

  auto maybe_value = evaluate(*expr.value);
  if (!maybe_value)
    return maybe_value;

  return {res->get<evaluation::Instance>().set_field(
      expr.field.to_string(kDetailed), *std::move(maybe_value))};
}
auto interpreter::visit2(const expression::This &expr) -> eval_result_t {
  return find_variable(expr.shared_from_this(), expr.name);
}
auto interpreter::visit2(const expression::Super &expr) -> eval_result_t {
  auto it = local_env.find(expr.shared_from_this());

  contract_assert(
      it != local_env.end(),
      "super class should be in local env; this shall be resolved in Resolver")

  const auto depth = it->second;
  
  auto superclass_ptr =
      env->get_at_depth(depth, "super")->get_if<evaluation::Class>();
  if (!superclass_ptr) {
    dbg(info, "environment: {}", env->to_string(kDetailed))
    return {auxilia::NotFoundError("Superclass not found in the environment.")};
  }
  dbg(info, "superclass name: {}", superclass_ptr->to_string(kDetailed))

  auto object_ptr =
      env->get_at_depth(depth - 1, "this")->get_if<evaluation::Instance>();
  if (!object_ptr) {
    dbg(info, "environment: {}", env->to_string(kDetailed))
    return {auxilia::NotFoundError(
        "Superclass must be called from a subclass instance.")};
  }
  dbg(info, "object name: {}", object_ptr->to_string(kDetailed))
  
  // clang-format off
  return superclass_ptr
      ->get_method(expr.method.to_string(kDetailed))
      .transform([&](auto &&method) -> variant_type {
        return {method.bind(*object_ptr)};
      });
  // clang-format on
}
#pragma endregion expression
#pragma region utility
auto interpreter::expr_to_string(
    const auxilia::FormatPolicy &format_policy) const -> string_type {
  return value_to_string(format_policy, last_expr_res);
}
auto interpreter::value_to_string(const auxilia::FormatPolicy &format_policy,
                                  const eval_result_t &value) const
    -> string_type {
  return value->to_string(format_policy);
}
auto interpreter::to_string(const auxilia::FormatPolicy &format_policy) const
    -> string_type {
  dbg(info, "last_expr_res index: {}", last_expr_res->index())
  dbg(info, "stmts size: {}", stmts_res.size())
  if (stmts_res.empty()) { // we are parse an expression, not a statement
    if (last_expr_res->index() && !is_interpreting_stmts)
      return value_to_string(format_policy, last_expr_res);
    return {};
  }

  string_type result_str;
  // clang-format off
  std::ranges::for_each(
      stmts_res 
      | std::ranges::views::transform([&](const auto &res) {
        return value_to_string(format_policy, res);
      }) 
      | std::ranges::views::filter([](const auto &str) {
        // FIXME: temporary solution: skip newline if the result is empty(i.e., Monostate)
        return !str.empty();
      }),
      [&](const auto &str) { 
        result_str += std::move(str) + '\n'; 
  });
  // clang-format on
  return result_str;
}
evaluation::Boolean
interpreter::is_true_value(const eval_result_t &value) const {
  return value->visit(match(
      [](const evaluation::Nil &n) {
        return evaluation::Boolean::make_false(n.get_line());
      },
      [](const evaluation::Boolean &b) { return b; },
      [](const auto &) { return evaluation::True; }));
}
auto interpreter::is_deep_equal(const eval_result_t &lhs,
                                const eval_result_t &rhs) const
    -> evaluation::Boolean {
  using Bool = evaluation::Boolean;
  auto pattern = match(
      [](const evaluation::Nil &l, const evaluation::Nil &r) -> Bool {
        return {Bool::make_true(l.get_line())};
      },
      []<typename T>(const T &l, const T &r) -> Bool { return {l == r}; },
      [](const auto &l, const auto &r) -> Bool {
        return {Bool{evaluation::False}};
      });
  return auxilia::visit(pattern, *lhs, *rhs);
}
auto interpreter::get_call_args(const expression::Call &expr) const
    -> auxilia::StatusOr<std::vector<variant_type>> {
  auto args = std::vector<variant_type>{};
  args.reserve(expr.args.size());

  // we choose to evaluate argument expressions from left to right, adhering
  // to "Sequenced before" rules in C++11 and later.
  // https://en.cppreference.com/w/cpp/language/eval_order
  for (const auto &arg : expr.args) {
    auto res = evaluate(*arg);
    if (!res)
      return {res.as_status()};
    args.emplace_back(*res);
  }
  return {args};
}
auto interpreter::get_function(const statement::Function &stmtFunc,
                               const bool is_initializer)
    -> evaluation::Function {
  // clang-format off
  return evaluation::Function::create_custom(
    stmtFunc.parameters.size(),
    {
      .name = stmtFunc.name.to_string(kDetailed),
      .parameters = stmtFunc.parameters
                    | std::ranges::views::transform([&](const auto &param) {
                        return param.to_string(kDetailed);
                      })
                    | std::ranges::to<std::vector<string_type>>(),
      .body = stmtFunc.body.statements
    },
    this->env,
    is_initializer);
  // clang-format on
}
auto interpreter::find_variable(const cexpr_ptr_t &expr, const Token &name)
    -> eval_result_t {
  if (auto it = local_env.find(expr); it != local_env.end()) {
    return *env->get_at_depth(it->second, name.to_string(kDetailed));
  }
  if (auto res = Environment::Global()->get(name.to_string(kDetailed));
      res && !res->empty()) {
    return *res;
  }

  return {auxilia::NotFoundError("Undefined variable '{}'.\n[line {}]",
                                 name.to_string(kDetailed),
                                 name.line)};
}
AC_LOX_API void delete_interpreter_fwd(interpreter *ptr) { delete ptr; }
#pragma endregion utility
} // namespace accat::lox
