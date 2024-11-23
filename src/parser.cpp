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
  expr_head = expression();
  return utils::OkStatus();
}
auto parser::get_expr() const -> expr_ptr_t {

  // default to add a group as the root node
  if (!expr_head)
    return std::make_shared<Grouping>(
        std::make_shared<Literal>(Token{TokenType::kNil, "nil"sv, 0.0l}));
  // auto group = std::make_shared<Grouping>(expr_head);
  // return group;
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
    equalityExpr = std::make_shared<Binary>(op, equalityExpr, right);
  }
  return equalityExpr;
}
auto parser::comparison() -> expr_ptr_t { // NOLINT(misc-no-recursion)
  auto comparisonExpr = term();
  while (inspect(kGreater, kGreaterEqual, kLess, kLessEqual)) {
    auto op = get();
    auto right = term();
    comparisonExpr = std::make_shared<Binary>(op, comparisonExpr, right);
  }
  return comparisonExpr;
}
auto parser::term() -> expr_ptr_t { // NOLINT(misc-no-recursion)
  auto termExpr = factor();
  while (inspect(kMinus, kPlus)) {
    auto op = get();
    auto right = factor();
    termExpr = std::make_shared<Binary>(op, termExpr, right);
  }
  return termExpr;
}
auto parser::factor() -> expr_ptr_t { // NOLINT(misc-no-recursion)
  auto factorExpr = unary();
  while (inspect(kSlash, kStar)) {
    auto op = get();
    auto right = unary();
    factorExpr = std::make_shared<Binary>(op, factorExpr, right);
  }
  return factorExpr;
}
auto parser::unary() -> expr_ptr_t { // NOLINT(misc-no-recursion)
  if (inspect(kBang, kMinus)) {
    auto op = get();
    auto right = unary();
    return std::make_shared<Unary>(op, right);
  }
  return primary();
}
auto parser::primary() -> expr_ptr_t { // NOLINT(misc-no-recursion)
  // TODO: implement
  if (inspect(kFalse))
    return std::make_shared<Literal>(get());
  if (inspect(kTrue))
    return std::make_shared<Literal>(get());
  if (inspect(kNil))
    return std::make_shared<Literal>(get());
  if (inspect(kNumber))
    return std::make_shared<Literal>(get());
  if (inspect(kString))
    return std::make_shared<Literal>(get());
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
      contract_assert(false);
    }
    get();
    return std::make_shared<Grouping>(expr);
  }
  dbg(critical, "Not implemented.");
  contract_assert(false);
  return nullptr;
}
} // namespace net::ancillarycat::loxograph
