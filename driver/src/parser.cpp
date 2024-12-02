#include <memory>

#include "config.hpp"
#include "expression.hpp"
#include "utils.hpp"
#include "loxo_fwd.hpp"

#include "parser.hpp"
namespace net::ancillarycat::loxograph {

parser &parser::set_views(const token_views_t tokens) {
  contract_assert(
      tokens.back().type.type, kEndOfFile, "tokens must end with EOF");
  this->tokens = tokens;
  this->cursor = tokens.begin();
  return *this;
}
bool parser::is_at_end(const size_type offset) const {
  // return (current + offset > tokens.size()) ||
  //        tokens[current].type.type == kEndOfFile;

  // return cursor + offset >= tokens.end();
  /// @note: ^^^^^^ MSVC has iterator assertion on whether the iterator is past
  /// the end(that operator `+` just has this assertion), be careful.
  ///     so we just use `std::ranges::distance` to check if the distance is
  ///     less than the offset.
  return std::ranges::distance(cursor, tokens.end()) <= offset ||
         cursor->type.type == kEndOfFile;
}
auto parser::get(const size_type offset) -> token_t {
  // contract_assert(current < tokens.size());
  // auto &token = tokens[current];
  // current += offset;
  // return token;
  contract_assert(cursor < tokens.end());
  auto &token = *cursor;
  cursor += offset;
  return token;
}
auto parser::parse(const ParsePolicy &parse_policy) -> utils::Status try {
  if (parse_policy == kExpression) {
    expr_head = next_expression();
  } else if (parse_policy == kStatement) {
    while (not is_at_end()) {
      stmts.emplace_back(next_declaration());
    }
  } else {
    contract_assert(false);
    dbg(critical, "unreachable code reached: {}", LOXOGRAPH_STACKTRACE);
  }
  return utils::OkStatus();
} catch (const expr_ptr_t &expr) {
  return utils::Status{utils::Status::kParseError, expr->to_string()};
}

auto parser::get_statements() const -> stmt_ptrs_t & {
  contract_assert(not stmts.empty(),
                  1,
                  "statements are empty; maybe you called `parse(kExpression)` "
                  "but not `parse(kStatement)`?");
  return stmts;
}
auto parser::get_expression() const -> expr_ptr_t & {
  contract_assert(expr_head != nullptr,
                  1,
                  "expression is null; maybe you called `parse(kStatement)` "
                  "but not `parse(kExpression)`?");
  return expr_head;
}
auto parser::next_expression() -> expr_ptr_t { // NOLINT(misc-no-recursion)
  return assignment();
}
auto parser::assignment() -> expr_ptr_t {
  auto expr = equality();
  if (inspect(kEqual)) {
    auto eq_op = get();
    auto res = assignment();
    if (auto var_name = std::dynamic_pointer_cast<expression::Variable>(expr)) {
      return std::make_shared<expression::Assignment>(var_name->name, res);
    }
    contract_assert(false, 0, "expect a variable in assignment.");
    throw synchronize({parse_error::kUnknownError, "Expect variable name."});
  }
  return expr;
}
auto parser::equality() -> expr_ptr_t { // NOLINT(misc-no-recursion)
  auto equalityExpr = comparison();
  while (inspect(kEqualEqual, kBangEqual)) {
    auto op = get();
    auto right = comparison();
    equalityExpr =
        std::make_shared<expression::Binary>(op, equalityExpr, right);
  }
  return equalityExpr;
}
auto parser::comparison() -> expr_ptr_t { // NOLINT(misc-no-recursion)
  auto comparisonExpr = term();
  while (inspect(kGreater, kGreaterEqual, kLess, kLessEqual)) {
    auto op = get();
    auto right = term();
    comparisonExpr =
        std::make_shared<expression::Binary>(op, comparisonExpr, right);
  }
  return comparisonExpr;
}
auto parser::term() -> expr_ptr_t { // NOLINT(misc-no-recursion)
  auto termExpr = factor();
  while (inspect(kMinus, kPlus)) {
    auto op = get();
    auto right = factor();
    termExpr = std::make_shared<expression::Binary>(op, termExpr, right);
  }
  return termExpr;
}
auto parser::factor() -> expr_ptr_t { // NOLINT(misc-no-recursion)
  auto factorExpr = unary();
  while (inspect(kSlash, kStar)) {
    auto op = get();
    auto right = unary();
    factorExpr = std::make_shared<expression::Binary>(op, factorExpr, right);
  }
  return factorExpr;
}
auto parser::unary() -> expr_ptr_t { // NOLINT(misc-no-recursion)
  if (inspect(kBang, kMinus)) {
    auto op = get();
    auto right = unary();
    return std::make_shared<expression::Unary>(op, right);
  }
  return primary();
}
auto parser::primary() -> expr_ptr_t { // NOLINT(misc-no-recursion)
  if (inspect(kFalse))
    return std::make_shared<expression::Literal>(get());
  if (inspect(kTrue))
    return std::make_shared<expression::Literal>(get());
  if (inspect(kNil))
    return std::make_shared<expression::Literal>(get());
  if (inspect(kNumber))
    return std::make_shared<expression::Literal>(get());
  if (inspect(kString))
    return std::make_shared<expression::Literal>(get());
  if (inspect(kIdentifier)) {
    return std::make_shared<expression::Variable>(get());
  }
  ///  where's keyword??????????????
  ///     ^^^^^^ solved: shoud not appera here and was already handled in lexer.
  // {
  // // codecafter's test does not need quotes around strings, so remove them
  // auto token = get();
  // token.lexeme = token.lexeme.substr(1, token.lexeme.size() - 2);
  // return std::make_shared<Literal>(token);
  //
  // }
  if (inspect(kLeftParen)) {
    get();
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
      //   get();
      // } else {
      //   dbg(critical, "unreachable code reached: {}", LOXOGRAPH_STACKTRACE);
      //   contract_assert(false);
      // }
      throw synchronize(
          {parse_error::kMissingParenthesis, "Expect expression."});
    }
    get();
    return std::make_shared<expression::Grouping>(expr);
  }
  // invalid evaluation reached
  throw synchronize({parse_error::kUnknownError, "Expect expression."});
}
auto parser::next_declaration() -> stmt_ptr_t {
  if (inspect(kVar)) {
    get();
    return var_decl();
  }
  return next_statement();
}
auto parser::var_decl() -> stmt_ptr_t {

  if (peek().type.type != kIdentifier) {
    contract_assert(false);
    TODO("not implemented");
  }
  auto var_tok = get();
  expr_ptr_t initializer = nullptr;
  if (inspect(kEqual)) {
    get();
    initializer = next_expression();
  }
  if (inspect(kSemicolon)) {
    get();
    return std::make_shared<statement::Variable>(std::move(var_tok),
                                                 initializer);
  }
  throw synchronize({parse_error::kUnknownError, "Expect expression."});
}
auto parser::print_stmt() -> stmt_ptr_t {
  auto value = next_expression();
  if (inspect(kSemicolon)) {
    get();
    return std::make_shared<statement::Print>(value);
  }
  throw synchronize({parse_error::kUnknownError, "Expect expression."});
}
auto parser::expr_stmt() -> stmt_ptr_t {
  auto expr = next_expression();
  if (inspect(kSemicolon)) {
    get();
    return std::make_shared<statement::Expression>(expr);
  }
  throw synchronize({parse_error::kUnknownError, "Expect expression."});
}
auto parser::next_statement() -> stmt_ptr_t {
  if (inspect(kPrint)) {
    get();
    return print_stmt();
  }
  if (inspect(kLeftBrace)) {
    get();
    stmt_ptrs_t statements;
    while (!inspect(kRightBrace) && !is_at_end()) {
      statements.emplace_back(next_declaration());
    }
    if (inspect(kRightBrace)) {
      get();
      return std::make_shared<statement::Block>(statements);
    }
    throw synchronize({parse_error::kMissingBrace, "Expect '}'."});
  }
  return expr_stmt();
}
auto parser::synchronize(const parse_error &parse_error) -> expr_ptr_t {
  /// advance until we have a semicolon
  // cueerntly cursor is at the error token: peek() returns the error token,
  // get() returns the error token and advances the cursor
  auto error_token = get();
  dbg(warn,
      "error at '{}'",
      error_token.to_string(utils::FormatPolicy::kTokenOnly));
  auto error_expr =
      std::make_shared<expression::IllegalExpr>(error_token, parse_error);
  while (!is_at_end() && !inspect(kSemicolon)) {
    dbg_block(auto discarded_token = peek();
              dbg(warn, "discarding {}", discarded_token););
    get();
  }
  return error_expr;
}
LOXOGRAPH_API void delete_parser_fwd(parser *ptr) { delete ptr; }
} // namespace net::ancillarycat::loxograph
