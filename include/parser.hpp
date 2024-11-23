#pragma once
#include <cstdint>
#include <memory>
#include <type_traits>
#include <vector>
#include "config.hpp"
#include "Expr.hpp"
#include "spdlog/spdlog.h"
#include "status.hpp"
#include "Token.hpp"
#include "TokenType.hpp"

namespace net::ancillarycat::loxograph {
enum class Precedence : uint8_t {
  kNone = 0,
  kEquality = 1,
  kComparison = 2,
  // from math: a term is a single part of an expression that is separated by
  // addition or subtraction
  kTerm = 3,
  // ditto: a factor is a number or variable that is multiplied within a term
  kFactor = 4,
  kUnary = 5,
  kCall = 6,
  kPrimary = 7,
};
class parser {
public:
  using token_t = Token;
  using token_views_t = std::span<token_t>;
  using token_type_t = Token::token_type;
  using string_type = Token::string_type;
  using string_view_type = token_t::string_view_type;
  using size_type = token_views_t::size_type;
  using ssize_type = decltype(std::ssize(std::declval<token_views_t>()));
  using expr_t = Expr;
  using expr_ptr_t = std::shared_ptr<expr_t>;

public:
  parser() = default;
  /// @note take ownership
  parser &set_views(const token_views_t &tokens = {}) {
    this->tokens = tokens;
    return *this;
  }

private:
  using enum token_type_t::type_t;

  bool is_at_end(const size_type offset = 0) const {
    return current + offset >= tokens.size();
  }
  /// @note i dont want to write 4 different constness-related overloads. so
  /// just use deducing this.
  auto peek(this auto &&self, const ssize_type offset = 0) -> decltype(auto) {
    if (self.is_at_end(offset))
      return self.tokens.back();
    return self.tokens[self.current + offset];
  }
  auto get(const size_type offset = 1) -> decltype(auto) {
    contract_assert(current < tokens.size());
    auto &token = tokens[current];
    current += offset;
    return token;
  }

public:
  /// @brief main entry point for parsing.
  /// @note Expression needs to be shared; especially for variables.
  ///  `(a + b) * (a + b)`. `(a + b)` is shared. `std::unique_ptr` may
  /// bring redundancy.
  auto parse() -> utils::Status {
    expr_head = expression();
    return utils::OkStatus();
  }
  auto get_expr() const -> expr_ptr_t {

    // default to add a group as the root node
    if (!expr_head)
      return std::make_shared<Grouping>(
          std::make_shared<Literal>(Token{TokenType::kNil, "nil"sv, 0.0l}));
    // auto group = std::make_shared<Grouping>(expr_head);
    // return group;
    return expr_head;
  }

private:
  /// @note euqality has the lowest precedence
  auto expression() -> expr_ptr_t { // NOLINT(misc-no-recursion)
    return equality();
  }
  /// @note equality has the second lowest precedence;
  ///			comparison generates equality.
  auto equality() -> expr_ptr_t { // NOLINT(misc-no-recursion)
    auto equalityExpr = comparison();
    while (inspect(kEqualEqual, kBangEqual)) {
      auto op = get();
      auto right = comparison();
      equalityExpr = std::make_shared<Binary>(op, equalityExpr, right);
    }
    return equalityExpr;
  }
  /// @note dittos
  auto comparison() -> expr_ptr_t { // NOLINT(misc-no-recursion)
    auto comparisonExpr = term();
    while (inspect(kGreater, kGreaterEqual, kLess, kLessEqual)) {
      auto op = get();
      auto right = term();
      comparisonExpr = std::make_shared<Binary>(op, comparisonExpr, right);
    }
    return comparisonExpr;
  }
  /// @note dittos
  auto term() -> expr_ptr_t { // NOLINT(misc-no-recursion)
    auto termExpr = factor();
    while (inspect(kMinus, kPlus)) {
      auto op = get();
      auto right = factor();
      termExpr = std::make_shared<Binary>(op, termExpr, right);
    }
    return termExpr;
  }
  // /// @note dittos
  auto factor() -> expr_ptr_t { // NOLINT(misc-no-recursion)
    auto factorExpr = unary();
    while (inspect(kSlash, kStar)) {
      auto op = get();
      auto right = unary();
      factorExpr = std::make_shared<Binary>(op, factorExpr, right);
    }
    return factorExpr;
  }
  // /// @note dittos
  auto unary() -> expr_ptr_t { // NOLINT(misc-no-recursion)
    if (inspect(kBang, kMinus)) {
      auto op = get();
      auto right = unary();
      return std::make_shared<Unary>(op, right);
    }
    return primary();
  }
  // /// @note dittos
  // auto call() ->expr_ptr_t {} // todo: implement
  // /// @note dittos
  auto primary() -> expr_ptr_t { // NOLINT(misc-no-recursion)
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

private:
  template <typename... Args>
    requires(std::is_enum_v<std::common_type_t<Args...>>)
  bool inspect(Args &&...args) {
    if (is_at_end())
      return false;
    return (... || (peek().type.type == args));
  }

private:
  token_views_t tokens;
  size_type current = 0;
  expr_ptr_t expr_head = nullptr;
};
} // namespace net::ancillarycat::loxograph
