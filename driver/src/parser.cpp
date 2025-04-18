#include <algorithm>
#include <iterator>
#include <memory>
#include <utility>
#include <vector>

#include <accat/auxilia/auxilia.hpp>

#include "Token.hpp"
#include "details/lox_fwd.hpp"

#include "statement.hpp"
#include "expression.hpp"

#include "parser.hpp"
namespace accat::lox {
// NOLINTBEGIN(misc-no-recursion)
parser &parser::set_views(const token_views_t tokens) {
  contract_assert(tokens.size() && tokens.back().is_type(kEndOfFile),
                  "tokens must have at least 1 token and ends with EOF")
  this->tokens = tokens;
  this->cursor = tokens.begin();
  return *this;
}
bool parser::is_at_end(const size_type offset) const {
  // return cursor + offset >= tokens.end();
  /// @note: ^^^^^^ MSVC has iterator assertion on whether the iterator is past
  /// the end(that operator `+` just has this assertion), be careful.
  ///     so we just use `std::ranges::distance` to check if the distance is
  ///     less than the offset.
  return std::ranges::distance(cursor, tokens.end()) <= offset ||
         cursor->is_type(kEndOfFile);
}
auto parser::get(const size_type offset) -> token_t {
  contract_assert(cursor < tokens.end(), "cursor out of range")
  auto &token = *cursor;
  cursor += offset;
  return token;
}
auto parser::parse(const ParsePolicy &parse_policy) -> auxilia::Status try {
  if (parse_policy == kExpression) {
    expr_head = next_expression();
  } else if (parse_policy == kStatement) {
    while (not is_at_end()) {
      stmts.emplace_back(next_declaration());
    }
  } else {
    contract_assert(false, "unknown parse policy.")
  }
  return {};
} catch (const auxilia::Status &status) {
  return status;
}

auto parser::get_statements() const -> stmt_ptrs_t & {
  contract_assert(not stmts.empty(),

                  "statements are empty; maybe you called `parse(kExpression)` "
                  "but not `parse(kStatement)`?")
  return stmts;
}
auto parser::get_expression() const -> expr_ptr_t & {
  contract_assert(expr_head != nullptr,

                  "expression is null; maybe you called `parse(kStatement)` "
                  "but not `parse(kExpression)`?")
  return expr_head;
}
auto parser::next_expression() -> expr_ptr_t { return assignment(); }
auto parser::assignment() -> expr_ptr_t {
  auto expr = logical_or();
  if (inspect(kEqual)) {
    auto eq_op = this->get();
    auto res = assignment();
    if (auto variable = std::dynamic_pointer_cast<expression::Variable>(expr)) {
      return std::make_shared<expression::Assignment>(std::move(variable->name),
                                                      std::move(res));
    }
    if (auto get_expr = std::dynamic_pointer_cast<expression::Get>(expr)) {
      return std::make_shared<expression::Set>(std::move(get_expr->object),
                                               std::move(get_expr->field),
                                               std::move(res));
    }
    throw synchronize({parse_error::kUnknownError, "Expect variable name."});
  }
  return expr;
}
auto parser::logical_or() -> expr_ptr_t {
  auto expr = logical_and();
  while (inspect(kOr)) {
    auto or_op = this->get();
    auto rhs = logical_or();
    // FIXME: move myself and reassign it??? is it legal?
    expr = std::make_shared<expression::Logical>(
        std::move(or_op), std::move(expr), std::move(rhs));
  }
  return expr;
}
auto parser::logical_and() -> expr_ptr_t {
  auto expr = equality();
  while (inspect(kAnd)) {
    auto eq_op = this->get();
    auto rhs = equality();
    expr = std::make_shared<expression::Logical>(
        std::move(eq_op), std::move(expr), std::move(rhs));
  }
  return expr;
}
auto parser::equality() -> expr_ptr_t {
  auto equalityExpr = comparison();
  while (inspect(kEqualEqual, kBangEqual)) {
    auto op = this->get();
    auto rhs = comparison();
    equalityExpr = std::make_shared<expression::Binary>(
        std::move(op), std::move(equalityExpr), std::move(rhs));
  }
  return equalityExpr;
}
auto parser::comparison() -> expr_ptr_t {
  auto comparisonExpr = term();
  while (inspect(kGreater, kGreaterEqual, kLess, kLessEqual)) {
    auto op = this->get();
    auto rhs = term();
    comparisonExpr = std::make_shared<expression::Binary>(
        std::move(op), std::move(comparisonExpr), std::move(rhs));
  }
  return comparisonExpr;
}
auto parser::term() -> expr_ptr_t {
  auto termExpr = factor();
  while (inspect(kMinus, kPlus)) {
    auto op = this->get();
    auto rhs = factor();
    termExpr = std::make_shared<expression::Binary>(
        std::move(op), std::move(termExpr), std::move(rhs));
  }
  return termExpr;
}
auto parser::factor() -> expr_ptr_t {
  auto factorExpr = unary();
  while (inspect(kSlash, kStar)) {
    auto op = this->get();
    auto rhs = unary();
    factorExpr = std::make_shared<expression::Binary>(
        std::move(op), std::move(factorExpr), std::move(rhs));
  }
  return factorExpr;
}
auto parser::unary() -> expr_ptr_t {
  if (inspect(kBang, kMinus)) {
    auto op = this->get();
    auto rhs = unary();
    return std::make_shared<expression::Unary>(std::move(op), std::move(rhs));
  }
  return call();
}
auto parser::call() -> expr_ptr_t {
  auto expr = primary();

  while (true) {
    if (inspect(kLeftParen)) {
      auto paren = this->get();
      auto args = get_args();
      expr = std::make_shared<expression::Call>(
          std::move(expr), std::move(paren), std::move(args));
    } else if (inspect(kDot)) {
      this->get();
      if (!inspect(kIdentifier)) {
        throw synchronize(parse_error{parse_error::kUnknownError,
                                      "Expect property name after '.'."});
      }
      auto name = this->get();
      expr =
          std::make_shared<expression::Get>(std::move(expr), std::move(name));
    } else
      break;
  }

  return expr;
}
auto parser::primary() -> expr_ptr_t {
  if (inspect(kFalse))
    return std::make_shared<expression::Literal>(this->get());
  if (inspect(kTrue))
    return std::make_shared<expression::Literal>(this->get());
  if (inspect(kNil))
    return std::make_shared<expression::Literal>(this->get());
  if (inspect(kNumber))
    return std::make_shared<expression::Literal>(this->get());
  if (inspect(kString))
    return std::make_shared<expression::Literal>(this->get());
  if (inspect(kThis))
    return std::make_shared<expression::This>(this->get());
  if (inspect(kSuper)) {
    auto name = this->get();
    if (!inspect(kDot))
      throw synchronize(
          {parse_error::kUnknownError, "Expect '.' after 'super'."});

    this->get();
    if (!inspect(kIdentifier))
      throw synchronize(
          {parse_error::kUnknownError, "Expect property name after '.'."});

    return std::make_shared<expression::Super>(std::move(name), this->get());
  }
  if (inspect(kIdentifier)) {
    return std::make_shared<expression::Variable>(this->get());
  }
  if (inspect(kLeftParen)) {
    this->get();
    auto expr = next_expression();
    if (!inspect(kRightParen)) {
      // invalid evaluation reached
      // is_in_panic = true;
      // TODO vvvvvv
      // auto error_expr = synchronize(
      //     {parse_error::kMissingParenthesis, "Expect expression."});
      // if (is_at_end()) {
      //
      // } else if (inspect(kSemicolon)) {
      //   this->get();
      // } else {
      //   dbg(critical, "unreachable code reached: {}", lox_STACKTRACE);
      //   contract_assert(false);
      // }
      throw synchronize(
          {parse_error::kMissingParenthesis, "Expect expression."});
    }
    this->get();
    return std::make_shared<expression::Grouping>(std::move(expr));
  }
  // invalid evaluation reached
  throw synchronize({parse_error::kUnknownError, "Expect expression."});
}
auto parser::get_args() -> std::vector<expr_ptr_t> {
  std::vector<expr_ptr_t> args;
  if (!inspect(kRightParen))
    do {
      args.emplace_back(next_expression());
      if (args.size() > 255) {
        throw synchronize({parse_error::kUnknownError,
                           "Cannot have more than "
                           "255 arguments."});
      }
    } while (inspect(kComma) && (this->get(), true)); // FIXME: so ugly
  if (!inspect(kRightParen)) {
    throw synchronize({parse_error::kUnknownError, "Expect ')'."});
  }
  this->get(); // right paren
  return args;
}
auto parser::get_params() -> std::vector<token_t> {
  std::vector<token_t> params;
  if (!inspect(kRightParen))
    do {
      auto maybe_ident = this->get();
      if (!maybe_ident.is_type(kIdentifier)) {
        throw synchronize(
            {parse_error::kUnknownError, "Expect parameter name."});
      }
      params.emplace_back(std::move(maybe_ident));
      if (params.size() > 255) {
        throw synchronize({parse_error::kUnknownError,
                           "Cannot have more than "
                           "255 parameters."});
      }
    } while (inspect(kComma) && (this->get(), true));
  if (!inspect(kRightParen)) {
    throw synchronize({parse_error::kUnknownError, "Expect ')'."});
  }
  this->get(); // right paren
  return params;
}
auto parser::get_stmts() -> stmt_ptrs_t {
  stmt_ptrs_t statements;
  while (!inspect(kRightBrace) && !is_at_end()) {
    statements.emplace_back(next_declaration());
  }
  if (!inspect(kRightBrace)) {
    throw synchronize({parse_error::kMissingBrace, "Expect '}'."});
  }
  this->get();
  return statements;
}
auto parser::next_declaration() -> stmt_ptr_t {
  if (inspect(kVar)) {
    this->get();
    return var_decl();
  }
  if (inspect(kFun)) {
    this->get();
    return function_decl();
  }
  return next_statement();
}
auto parser::var_decl() -> stmt_ptr_t {

  if (!peek().is_type(kIdentifier)) {
    throw synchronize({parse_error::kUnknownError, "Expect variable name."});
  }
  auto var_tok = this->get();
  expr_ptr_t initializer = nullptr;
  if (inspect(kEqual)) {
    this->get();
    initializer = next_expression();
  }
  if (!inspect(kSemicolon)) {
    throw synchronize({parse_error::kUnknownError, "Expect expression."});
  }
  this->get();
  return std::make_shared<statement::Variable>(std::move(var_tok),
                                               std::move(initializer));
}
auto parser::function_decl_impl() -> statement::Function {
  auto name = this->get();

  if (!inspect(kLeftParen)) {
    throw synchronize({parse_error::kMissingParenthesis, "Expect '('."});
  }
  this->get();
  auto parameters = get_params();
  if (!inspect(kLeftBrace)) {
    throw synchronize({parse_error::kMissingBrace, "Expect '{'."});
  }
  this->get();
  auto body = get_stmts();
  return statement::Function(
      std::move(name), std::move(parameters), std::move(body));
}
auto parser::function_decl() -> stmt_ptr_t {
  return std::make_shared<statement::Function>(function_decl_impl());
}
auto parser::get_methods() -> std::vector<statement::Function> {
  std::vector<statement::Function> methods;
  // in lox grammar the `fun` keyword is not required when inside a class.
  while (
      // inspect(kFun) or
      !inspect(kRightBrace)) {
    // this->get();
    methods.emplace_back(function_decl_impl());
  }
  if (!inspect(kRightBrace)) {
    throw synchronize(
        {parse_error::kMissingBrace, "Expect '}' after class body."});
  }
  this->get();
  return methods;
}
auto parser::class_stmt() -> stmt_ptr_t {
  auto name = this->get();

  token_t superclass = token_t{};
  if (inspect(kLess)) {
    this->get();
    if (!inspect(kIdentifier)) {
      throw synchronize(
          {parse_error::kUnknownError, "Expect superclass name."});
    }
    superclass = this->get();
  }

  if (!inspect(kLeftBrace)) {
    throw synchronize(
        {parse_error::kMissingBrace, "Expect '{' before class body."});
  }
  this->get();
  return std::make_shared<statement::Class>(
      std::move(name),
      superclass.is_type(kIdentifier)
          ? std::make_shared<expression::Variable>(std::move(superclass))
          : nullptr,
      get_methods());
}
auto parser::get_condition() -> expr_ptr_t {
  if (!inspect(kLeftParen)) {
    throw synchronize({parse_error::kMissingParenthesis, "Expect '('."});
  }
  this->get();
  auto condition = next_expression();
  if (!inspect(kRightParen)) {
    throw synchronize({parse_error::kMissingParenthesis, "Expect ')'."});
  }
  this->get();
  return condition;
}
auto parser::if_stmt() -> stmt_ptr_t {
  auto condition = get_condition();
  auto then_branch = next_statement();
  stmt_ptr_t else_branch = nullptr;
  if (inspect(kElse)) {
    this->get();
    else_branch = next_statement();
  }
  return std::make_shared<statement::If>(
      std::move(condition), std::move(then_branch), std::move(else_branch));
}
auto parser::block_stmt() -> stmt_ptr_t {
  return std::make_shared<statement::Block>(get_stmts());
}
auto parser::while_stmt() -> stmt_ptr_t {
  auto condition = get_condition();
  auto body = next_statement();
  return std::make_shared<statement::While>(std::move(condition),
                                            std::move(body));
}
auto parser::for_stmt() -> stmt_ptr_t {
  if (!inspect(kLeftParen)) {
    throw synchronize({parse_error::kMissingParenthesis, "Expect '('."});
  }
  this->get();
  stmt_ptr_t initializer = nullptr;
  if (inspect(kVar)) {
    this->get();
    initializer = var_decl();
  } else if (inspect(kSemicolon)) {
    // consume the semicolon
    this->get();
  } else {
    initializer = expr_stmt();
  }
  /// @note ^^^^^^ actually C's grammar was more relaxed and allows for any
  ///   declaration or statement in the initializer part of the for loop.
  ///   here we only allow for variable declaration or expression statement.

  // else, no initializer
  // defaule condition is `true`.
  expr_ptr_t condition =
      std::make_shared<expression::Literal>(Token(kTrue, "true"sv, {"true"sv}));
  if (!inspect(kSemicolon)) {
    condition = next_expression();
  }
  if (!inspect(kSemicolon)) {
    throw synchronize({parse_error::kUnknownError, "Expect ';'."});
  }
  this->get();
  expr_ptr_t increment = nullptr;
  if (!inspect(kRightParen)) {
    increment = next_expression();
  }
  if (!inspect(kRightParen)) {
    throw synchronize({parse_error::kMissingParenthesis, "Expect ')'."});
  }
  this->get();
  auto body = next_statement();
  return std::make_shared<statement::For>(std::move(initializer),
                                          std::move(condition),
                                          std::move(increment),
                                          std::move(body));
}
auto parser::return_stmt() -> stmt_ptr_t {
  expr_ptr_t value = nullptr;
  auto line = std::ranges::prev(cursor)->line;

  if (!inspect(kSemicolon)) {
    value = next_expression();
  }
  if (!inspect(kSemicolon)) {
    throw synchronize({parse_error::kUnknownError, "Expect ';'."});
  }
  this->get();
  return std::make_shared<statement::Return>(std::move(value), line);
}
auto parser::print_stmt() -> stmt_ptr_t {
  auto value = next_expression();
  if (!inspect(kSemicolon)) {
    throw synchronize({parse_error::kUnknownError, "Expect expression."});
  }
  this->get();
  return std::make_shared<statement::Print>(std::move(value));
}
auto parser::expr_stmt() -> stmt_ptr_t {
  auto expr = next_expression();
  if (!inspect(kSemicolon)) {
    throw synchronize({parse_error::kUnknownError, "Expect expression."});
  }
  this->get();
  return std::make_shared<statement::Expression>(std::move(expr));
}
auto parser::next_statement() -> stmt_ptr_t {
  if (inspect(kPrint)) {
    this->get();
    return print_stmt();
  }
  if (inspect(kLeftBrace)) {
    this->get();
    return block_stmt();
  }
  if (inspect(kIf)) {
    this->get();
    return if_stmt();
  }
  if (inspect(kWhile)) {
    this->get();
    return while_stmt();
  }
  if (inspect(kFor)) {
    this->get();
    return for_stmt();
  }
  if (inspect(kClass)) {
    this->get();
    return class_stmt();
  }
  if (inspect(kReturn)) {
    this->get();
    return return_stmt();
  }
  return expr_stmt();
}
auto parser::synchronize(const parse_error &parse_error) -> auxilia::Status {
  /// advance until we have a semicolon
  // cueerntly cursor is at the error token: peek() returns the error token,
  // get() returns the error token and advances the cursor
  auto error_token = this->get();
  dbg(warn,
      "error at '{}'",
      error_token.to_string(auxilia::FormatPolicy::kDetailed))

  while (!is_at_end() && !inspect(kSemicolon)) {
    dbg_block
    {
      auto discarded_token = peek();
      dbg(warn, "discarding {}", discarded_token);
    };
    this->get();
  }
  return auxilia::ParseError(
      "[line {}] Error at '{}': {}",
      error_token.line,
      error_token.to_string(auxilia::FormatPolicy::kDetailed),
      parse_error.message());
}
AC_LOX_API void delete_parser_fwd(parser *ptr) { delete ptr; }

// NOLINTEND(misc-no-recursion)
} // namespace accat::lox
