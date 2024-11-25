#include "parser.hpp"
#include "config.hpp"
#include "loxo_fwd.hpp"

namespace net::ancillarycat::loxograph {

parser &parser::set_views(const token_views_t &tokens) {
  this->tokens = tokens;
  return *this;
}
bool parser::is_at_end(const size_type offset) const {
  return current + offset >= tokens.size();
}
auto parser::get(const size_type offset) -> token_t {
  contract_assert(current < tokens.size());
  auto &token = tokens[current];
  current += offset;
  return token;
}
auto parser::parse() -> utils::Status {
	// TODO: do not use exceptions
  try {
    expr_head = expression();
    return utils::OkStatus();
  } catch (const expr_ptr_t &expr) {
    return utils::Status{utils::Status::kParseError, expr->to_string()};
  }
}
auto parser::get_expr() const -> expr_ptr_t {
  if (!expr_head)
    return std::make_shared<expression::Grouping>(
        std::make_shared<expression::Literal>(Token{TokenType::kNil, "nil"sv, 0.0l}));
  return expr_head;
}
auto parser::expression() -> expr_ptr_t { // NOLINT(misc-no-recursion)
  return equality();
}
auto parser::equality() -> expr_ptr_t { // NOLINT(misc-no-recursion)
  auto equalityExpr = comparison();
  while (inspect(kEqualEqual, kBangEqual)) {
    auto op = get();
    auto right = comparison();
    equalityExpr = std::make_shared<expression::Binary>(op, equalityExpr, right);
  }
  return equalityExpr;
}
auto parser::comparison() -> expr_ptr_t { // NOLINT(misc-no-recursion)
  auto comparisonExpr = term();
  while (inspect(kGreater, kGreaterEqual, kLess, kLessEqual)) {
    auto op = get();
    auto right = term();
    comparisonExpr = std::make_shared<expression::Binary>(op, comparisonExpr, right);
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
  // {
  // // codecafter's test does not need quotes around strings, so remove them
  // auto token = get();
  // token.lexeme = token.lexeme.substr(1, token.lexeme.size() - 2);
  // return std::make_shared<Literal>(token);
  //
  // }
  if (inspect(kLeftParen)) {
    get();
    auto expr = expression();
    if (!inspect(kRightParen)) {
      // invalid syntax reached
      // is_in_panic = true;
      // TODO vvvvvv
      // auto error_expr = recovery_parse(
      //     {parse_error::kMissingParenthesis, "Expect expression."});
      // if (is_at_end()) {
      //
      // } else if (inspect(kSemicolon)) {
      //   get();
      // } else {
      //   dbg(critical, "unreachable code reached: {}", LOXOGRAPH_STACKTRACE);
      //   contract_assert(false);
      // }
      // auto _ = recovery_parse(
      //     {parse_error::kMissingParenthesis, "Expect expression."});
      // std::cerr << _->to_string() << std::endl;
			throw recovery_parse(
          {parse_error::kMissingParenthesis, "Expect expression."});
    }
    get();
    return std::make_shared<expression::Grouping>(expr);
  }
  // invalid syntax reached

  throw recovery_parse({parse_error::kUnknownError, "Expect expression."});
}
auto parser::recovery_parse(const parse_error &parse_error) -> expr_ptr_t {
  /// advance until we have a semicolon
  // cueerntly cursor is at the error token: peek() returns the error token,
  // get() returns the error token and advances the cursor
  auto error_token = get();
  dbg(warn, "error at {}", error_token);
  auto error_expr = std::make_shared<expression::IllegalExpr>(error_token, parse_error);
  while (!is_at_end() && !inspect(kSemicolon)) {
    dbg_block(auto discarded_token = peek();
              dbg(warn, "discarding {}", discarded_token););
    get();
  }
  return error_expr;
}
} // namespace net::ancillarycat::loxograph
