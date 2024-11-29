#include <memory>

#include "config.hpp"
#include "expression.hpp"
#include "fmt.hpp"
#include "loxo_fwd.hpp"

#include "parser.hpp"
namespace net::ancillarycat::loxograph {

parser &parser::set_views(const token_views_t tokens) {
  this->tokens = tokens;
  return *this;
}
bool parser::is_at_end(const size_type offset) const {
  return (current + offset > tokens.size()) ||
         tokens[current].type.type == kEndOfFile;
}
auto parser::get(const size_type offset) -> token_t {
  contract_assert(current < tokens.size());
  auto &token = tokens[current];
  current += offset;
  return token;
}
auto parser::parse(const ParsePolicy &parse_policy) -> utils::Status try {
  // TODO: do not use exceptions
  // TODO("implementing statement parsing, so code here WONT work!");
  if (parse_policy == kExpression) {
    expr_head = next_expression();
  } else if (parse_policy == kStatement) {
    while (not is_at_end()) {
      stmts.emplace_back(next_statement());
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
auto parser::get_expression() const -> expr_ptr_t {
  contract_assert(expr_head != nullptr,
                  1,
                  "expression is null; maybe you called `parse(kStatement)` "
                  "but not `parse(kExpression)`?");
  return expr_head;
}
// TODO: do not use exceptions for control flow
auto parser::next_expression() -> expr_ptr_t { // NOLINT(misc-no-recursion)
  return equality();
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
  // TODO: implement
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
  // TODO: where's keyword??????????????
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
auto parser::declaration() -> stmt_ptr_t {
  if (inspect(kVar)) {
    return var_decl();
  }
  return next_statement();
}
auto parser::var_decl() -> stmt_ptr_t {

  if (peek().type.type != kIdentifier) {
  }
  auto var_tok = get();
  expr_ptr_t initializer = nullptr;
  if (inspect_and_get(kEqual)) {
    initializer = next_expression();
  }
  if (inspect_and_get(kSemicolon)) {
    return std::make_shared<statement::Variable>(std::move(var_tok),
                                                 initializer);
  }
  TODO("Exception or return error?");
  // return std::make_shared<parse_error>{
  //     parse_error::kMissingSemicolon, "Expect ';' after variable
  //     declaration."};
}
auto parser::print_stmt() -> stmt_ptr_t {
  auto value = next_expression();
  if (inspect(kSemicolon)) {
    get();
    return std::make_shared<statement::Print>(value);
  }
  TODO("ditto");
  // return std::make_shared<parse_error>{parse_error::kMissingSemicolon,
  //                                      "Expect ';' after value."};
}
auto parser::expr_stmt() -> stmt_ptr_t {
  auto expr = next_expression();
  if (inspect(kSemicolon)) {
    get();
    return std::make_shared<statement::Expression>(expr);
  }
  // return std::make_shared<parse_error>{parse_error::kMissingSemicolon,"Expect
  // ';' after expression."};
  TODO("ditto");
}
auto parser::next_statement() -> stmt_ptr_t {
  if (inspect(kPrint)) {
    get();
    return print_stmt();
  }
  return expr_stmt();
}
auto parser::synchronize(const parse_error &parse_error) -> expr_ptr_t {
  /// advance until we have a semicolon
  // cueerntly cursor is at the error token: peek() returns the error token,
  // get() returns the error token and advances the cursor
  auto error_token = get();
  dbg(warn, "error at {}", error_token);
  auto error_expr =
      std::make_shared<expression::IllegalExpr>(error_token, parse_error);
  // TODO: temporary disable this
  while (!is_at_end() && !inspect(kSemicolon)) {
    dbg_block(auto discarded_token = peek();
              dbg(warn, "discarding {}", discarded_token););
    get();
  }
  return error_expr;
}
LOXOGRAPH_API void delete_parser_fwd(parser *ptr) { delete ptr; }
} // namespace net::ancillarycat::loxograph
