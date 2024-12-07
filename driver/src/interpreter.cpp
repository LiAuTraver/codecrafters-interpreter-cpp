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

#include "net/ancillarycat/utils/Status.hpp"
#include "net/ancillarycat/utils/config.hpp"
#include "net/ancillarycat/utils/format.hpp"

#include "details/loxo_fwd.hpp"
#include "Environment.hpp"
#include "Evaluatable.hpp"
#include "statement.hpp"
#include "expression.hpp"
#include "interpreter.hpp"

namespace net::ancillarycat::loxo {
using utils::match;
using enum TokenType::type_t;
interpreter::interpreter() : env(std::make_shared<Environment>()) {}
auto interpreter::interpret(
    const std::span<std::shared_ptr<statement::Stmt>> stmts) const
    -> stmt_result_t {
  is_interpreting_stmts = true;
  static bool has_init_global_env = false;
  if (!has_init_global_env) {
    auto maybe_env = Environment::getGlobalEnvironment();
    if (!maybe_env)
      return maybe_env.as_status();
    has_init_global_env = true;
    this->env = maybe_env.value();
    // this->global_env = env;
  }

  for (const auto &stmt : stmts)
    if (auto eval_res = execute(*stmt); !eval_res) {
      last_expr_res->clear();
      return eval_res;
    }

  return utils::OkStatus();
}

auto interpreter::set_env(const env_ptr_t &new_env) const
    -> const interpreter & {
  env = new_env;
  return *this;
}

evaluation::Boolean
interpreter::is_true_value(const eval_result_t &value) const {
  return value->visit(match{
      [](const evaluation::Nil &n) {
        return evaluation::Boolean::make_false(n.get_line());
      },
      [](const evaluation::Boolean &b) { return b; },
      [](const auto &) { return evaluation::True; },
  });
}
auto interpreter::is_deep_equal(const eval_result_t &lhs,
                                const eval_result_t &rhs) const
    -> evaluation::Boolean {
  return lhs->index() == rhs->index()
             ? lhs->visit(match{
                   [](const evaluation::Nil &n) -> evaluation::Boolean {
                     return {evaluation::Boolean::make_true(n.get_line())};
                   },
                   [&rhs](const evaluation::Boolean &b) -> evaluation::Boolean {
                     return {utils::get<evaluation::Boolean>(*rhs) == b};
                   },
                   [&rhs](const evaluation::String &s) -> evaluation::Boolean {
                     return {utils::get<evaluation::String>(*rhs) == s};
                   },
                   [&rhs](const evaluation::Number &n) -> evaluation::Boolean {
                     return {utils::get<evaluation::Number>(*rhs) == n};
                   },
                   [](const auto &) -> evaluation::Boolean {
                     return {evaluation::Boolean{evaluation::False}};
                   }})
             : evaluation::Boolean{evaluation::False};
}
auto interpreter::get_call_args(const expression::Call &expr) const
    -> utils::StatusOr<std::vector<variant_type>> {
  auto args = std::vector<variant_type>{};
  args.reserve(expr.args.size());

  // we choose to evaluate argument expressions from left to right, adhering
  // to "Sequenced before" rules in C++11 and later.
  // https://en.cppreference.com/w/cpp/language/eval_order
  for (const auto &arg : expr.args) {
    auto res = evaluate(*arg);
    if (!res)
      return {res};
    args.emplace_back(*res);
  }
  return {args};
}
auto interpreter::visit_impl(const statement::Variable &stmt) const
    -> stmt_result_t {

  if (stmt.has_initilizer()) {
    auto eval_res = evaluate(*stmt.initializer);
    if (!eval_res)
      return eval_res;
    contract_assert(!!std::any_cast<string_view_type>(&stmt.name.literal),
                    1,
                    "variable name should be a string")
    dbg(trace,
        "variable name: {}, value: {}",
        std::any_cast<string_view_type>(stmt.name.literal),
        eval_res->underlying_string())
    // string view failed again; not null-terminated
    return {env->add(
        stmt.name.to_string(utils::kTokenOnly), *eval_res, stmt.name.line)};
  }
  // if no initializer, it's a nil value.
  return env->add(stmt.name.to_string(utils::kTokenOnly),
                  evaluation::NilValue,
                  stmt.name.line);
}
auto interpreter::visit_impl(const statement::Print &stmt) const
    -> stmt_result_t {
  auto eval_res = evaluate(*stmt.value);
  if (!eval_res)
    return eval_res;
  stmts_res.emplace_back(*eval_res);
  // return utils::OkStatus();
  return {{evaluation::NilValue}};
}
auto interpreter::visit_impl(const statement::If &stmt) const -> stmt_result_t {
  auto eval_res = evaluate(*stmt.condition);
  if (!eval_res)
    return eval_res;
  if (is_true_value(*eval_res).is_true()) {
    // if (auto eval_res = execute(*stmt.then_branch); !eval_res)
    //   return eval_res;
    // return {*eval_res};
    return execute(*stmt.then_branch);
  }
  // else we execute the else branch.
  if (stmt.else_branch) { // maybe we dont have an else branch, so check it.
    // if (auto eval_res = execute(*stmt.else_branch); !eval_res)
    //   return eval_res;
    // return {*eval_res};
    return execute(*stmt.else_branch);
  }
  return {*eval_res};
}
auto interpreter::visit_impl(const statement::While &stmt) const
    -> stmt_result_t {
  stmt_result_t res;
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
auto interpreter::visit_impl(const statement::For &stmt) const
    -> stmt_result_t {

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
  return utils::OkStatus();
}
auto interpreter::visit_impl(const statement::Function &stmt) const
    -> stmt_result_t {
  // TODO: function overloading
  // clang-format off
  if (auto res = env->get(stmt.name.to_string(utils::kTokenOnly));
  !res.empty()){
    // FIXME: seems something went wrong with my logic here.
    dbg(warn, "found the function already defined... use it")
    return res;
  }
    // return {utils::InvalidArgument(utils::format(
    //     "Function '{}' already defined. \n"
    //     "Function overloading is not supported yet.",
    //     stmt.name.to_string(utils::kTokenOnly)))};

  dbg(info,"func name: {}",
      stmt.name.to_string(utils::kTokenOnly))

  // dbg(info,"env and parent: {}",
  //     env->parent == this->global_env? "global" : "local"
  //     )
      

  auto callable = evaluation::Callable::create_custom(
          stmt.parameters.size(),
          {stmt.name.to_string(utils::kTokenOnly),
           stmt.parameters
           | std::ranges::views::transform([&](const auto &param) {
               return param.to_string(utils::kTokenOnly);
             })
           | std::ranges::to<std::vector<string_type>>(),
           stmt.body.statements},
           this->env); 
  return env->add(
      stmt.name.to_string(utils::kTokenOnly),
      callable,
      stmt.name.line);
  // clang-format on
}
auto interpreter::visit_impl(const statement::Expression &stmt) const
    -> stmt_result_t {
  return evaluate(*stmt.expr);
}
auto interpreter::visit_impl(const statement::Block &stmt) const
    -> stmt_result_t {
  auto original_env = env; // save the original environment
  auto sub_env = std::make_shared<Environment>(env);
  env = sub_env;
  for (const auto &scoped_stmt : stmt.statements) {
    if (auto eval_res = execute(*scoped_stmt); !eval_res) {
      env = original_env; // restore // TODO: shall i also restore env when
                          // eval_res cantains `returning`?
      return eval_res;
    }
  }
  env = original_env; // restore
  return utils::OkStatus();
}
auto interpreter::execute_impl(const statement::Stmt &stmt) const
    -> stmt_result_t {
  return stmt.accept(*this);
}
auto interpreter::get_result_impl() const -> eval_result_t {
  return last_expr_res;
}

auto interpreter::evaluate_impl(const expression::Expr &expr) const
    -> stmt_result_t {
  auto res = expr.accept(*this);
  if (!res)
    return res;
  dbg(info, "result: {}", res->underlying_string())
  return last_expr_res.reset(*res);
}
auto interpreter::visit_impl(const statement::Return &expr) const
    -> stmt_result_t {
  if (not expr.value) {
    dbg(info, "returning nil")
    return Returning({{evaluation::NilValue}});
  }
  auto res = evaluate(*expr.value);
  dbg(info, "return value: {}", res->underlying_string())
  if (!res) {
    return res;
  }
  dbg(trace, "result: {}", res->underlying_string())
  return Returning(*res);
}
auto interpreter::visit_impl(const expression::Literal &expr) const
    -> eval_result_t {
  dbg(trace, "literal type: {}", expr.literal.type)
  if (expr.literal.is_type(kMonostate)) {
    dbg(critical, "should not happen.")
    contract_assert(false)
    return {};
  }
  if (expr.literal.is_type(kNil)) {
    return {evaluation::Nil{expr.literal.line}};
  }
  if (expr.literal.is_type(kTrue)) {
    return {evaluation::Boolean{true, expr.literal.line}};
  }
  if (expr.literal.is_type(kFalse)) {
    return {evaluation::Boolean{false, expr.literal.line}};
  }
  if (expr.literal.is_type(kString)) {
    return {evaluation::String{
        std::any_cast<string_view_type>(expr.literal.literal),
        expr.literal.line}};
  }
  if (expr.literal.is_type(kNumber)) {
    return {evaluation::Number{std::any_cast<long double>(expr.literal.literal),
                               expr.literal.line}};
  }
  return {utils::InvalidArgument(
      utils::format("Expected literal value.\n[line {}]", expr.literal.line))};
}

auto interpreter::visit_impl(const expression::Unary &expr) const
    -> eval_result_t {
  auto inner_expr = expr.expr->accept(*this);
  if (expr.op.is_type(kMinus)) {
    if (utils::holds_alternative<evaluation::Number>(inner_expr.value())) {
      auto value = utils::get<evaluation::Number>(inner_expr.value());
      dbg(trace, "unary minus: {}", value)
      return {evaluation::Number{value * (-1)}};
    }
    return {utils::InvalidArgument(
        utils::format("Operand must be a number.\n[line {}]", expr.op.line))};
  }
  if (expr.op.is_type(kBang)) {
    auto value = is_true_value(inner_expr.value());
    dbg(trace, "unary bang: {}", value)
    return {evaluation::Boolean{!value}};
  }
  contract_assert(false, 1, "unreachable code reached")
  return {utils::Monostate{}};
}

auto interpreter::visit_impl(const expression::Binary &expr) const
    -> eval_result_t {
  auto lhs = expr.left->accept(*this);
  auto rhs = expr.right->accept(*this);
  if (!lhs) {
    return lhs;
  }
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
    return {utils::InvalidArgument(
        utils::format("Operands must be two numbers or two strings.\n[line "
                      "{}]",
                      expr.op.line))};
  }
  if (utils::holds_alternative<evaluation::String>(*lhs)) {
    if (expr.op.is_type(kPlus)) {
      return {evaluation::String{utils::get<evaluation::String>(*lhs) +
                                 utils::get<evaluation::String>(*rhs)}};
    }
  }
  if (utils::holds_alternative<evaluation::Number>(*lhs)) {
    auto real_lhs = utils::get<evaluation::Number>(*lhs);
    auto real_rhs = utils::get<evaluation::Number>(*rhs);
    switch (expr.op.type.type) {
    case kMinus:
      return {evaluation::Number{real_lhs - real_rhs}};
    case kPlus:
      return {evaluation::Number{real_lhs + real_rhs}};
    case kSlash:
      return {evaluation::Number{real_lhs / real_rhs}};
    case kStar:
      return {evaluation::Number{real_lhs * real_rhs}};
    case kGreater:
      return {evaluation::Boolean{real_lhs > real_rhs}};
    case kGreaterEqual:
      return {evaluation::Boolean{real_lhs >= real_rhs}};
    case kLess:
      return {evaluation::Boolean{real_lhs < real_rhs}};
    case kLessEqual:
      return {evaluation::Boolean{real_lhs <= real_rhs}};
    default:
      break;
    }
  }
  dbg(error, "unimplemented binary operator: {}", expr.op.to_string())
  contract_assert(false)
  return {utils::InvalidArgument(utils::format(
      "unimplemented binary operator.\n[line {}]", expr.op.line))};
}
auto interpreter::visit_impl(const expression::Grouping &expr) const
    -> eval_result_t {
  return {expr.expr->accept(*this)};
}
auto interpreter::visit_impl(const expression::Variable &expr) const
    -> eval_result_t {
  if (auto res = env->get(expr.name.to_string(utils::FormatPolicy::kTokenOnly));
      !res.empty())
    return res;

  return {utils::NotFoundError(
      utils::format("Undefined variable '{}'.\n[line {}]",
                    expr.name.to_string(utils::FormatPolicy::kTokenOnly),
                    expr.name.line))};
}
auto interpreter::visit_impl(const expression::Assignment &expr) const
    -> eval_result_t {
  // FIXME: here my logic went away. fixme here.
  auto res = this->evaluate(*expr.value_expr);
  if (!res)
    return res;

  if (!env->reassign(expr.name.to_string(utils::FormatPolicy::kTokenOnly),
                     *res,
                     expr.name.line))
    return {utils::NotFoundError(
        utils::format("Undefined variable '{}'.\n[line {}]",
                      expr.name.to_string(utils::FormatPolicy::kTokenOnly),
                      expr.name.line))};
  return *res;
}
auto interpreter::visit_impl(const expression::Logical &expr) const
    -> eval_result_t {
  auto lhs = expr.left->accept(*this);
  if (!lhs)
    return lhs;
  if (is_true_value(*lhs).is_true()) {
    if (expr.op.is_type(kOr))
      return {*lhs};
    if (expr.op.is_type(kAnd))
      return {expr.right->accept(*this)};
    contract_assert(false, 1, "unimplemented logical operator")
    return {utils::Monostate{}};
  }
  // left is false, evaluate right.
  if (expr.op.is_type(kOr))
    return {expr.right->accept(*this)};
  if (expr.op.is_type(kAnd))
    return {evaluation::Boolean{false, expr.op.line}};
  contract_assert(false, 1, "unimplemented logical operator")
  return {utils::Monostate{}};
}
auto interpreter::visit_impl(const expression::Call &expr) const
    -> eval_result_t {
  auto res = evaluate(*expr.callee);
  if (!res)
    return res;

  // `result` would change in `get_call_args`, so we need to save it.
  const auto callee = expr.callee;
  if (!utils::holds_alternative<evaluation::Callable>(*res))
    return {utils::NotFoundError(utils::format("Not a function: {}\n[line {}]",
                                               callee->to_string(),
                                               expr.paren.line))};

  auto callable = utils::get<evaluation::Callable>(*res);
  const auto maybe_args = get_call_args(expr);

  if (!maybe_args)
    return {maybe_args.as_status()};

  if (maybe_args->size() == callable.arity())
    // clear `Returning` status has already been implemented in `call` method.
    // just return here.
    return callable.call(*this, *maybe_args);

  return {utils::InvalidArgument(
      maybe_args->size() > callable.arity()
          ? utils::format("Too many arguments to call function '{}': "
                          "expected {} but got {}",
                          callee->to_string(),
                          callable.arity(),
                          maybe_args->size())
          : utils::format("Too few arguments to call function '{}': "
                          "expected {} but got {}",
                          callee->to_string(),
                          callable.arity(),
                          maybe_args->size()))};
}

auto interpreter::expr_to_string(const utils::FormatPolicy &format_policy) const
    -> string_type {
  return value_to_string(format_policy, last_expr_res);
}
auto interpreter::value_to_string(const utils::FormatPolicy &format_policy,
                                  const eval_result_t &value) const
    -> string_type {
  return value->underlying_string(format_policy);
}
auto interpreter::to_string_impl(const utils::FormatPolicy &format_policy) const
    -> string_type {
  dbg(info, "last_expr_res index: {}", last_expr_res->index())
  dbg(info, "stmts size: {}", stmts_res.size())
  if (stmts_res.empty()) { // we are parse an expression, not a statement
    if (last_expr_res->index() && !is_interpreting_stmts)
      return value_to_string(format_policy, last_expr_res);
    else
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
        // FIXME: temporary solution: skip newline if the result is empty(i.e.,Monostate)
        return !str.empty();
      }),
      [&](const auto &str) { 
        result_str += std::move(str) + '\n'; 
  });
  // clang-format on
  return result_str;
}
LOXO_API void delete_interpreter_fwd(interpreter *ptr) { delete ptr; }
} // namespace net::ancillarycat::loxo
