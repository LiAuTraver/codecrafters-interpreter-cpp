#pragma once
#include <std.hh>
#include "config.hpp"
#include "Expr.hpp"
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
class LOXOGRAPH_API parser {
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
  using enum token_type_t::type_t;

public:
  parser() = default;
  /// @note take ownership
  parser &set_views(const token_views_t &tokens = {});

public:
  /// @brief main entry point for parsing.
  /// @note Expression needs to be shared; especially for variables.
  ///  `(a + b) * (a + b)`. `(a + b)` is shared. `std::unique_ptr` may
  /// bring redundancy.
  auto parse() -> utils::Status;
  auto get_expr() const -> expr_ptr_t;

private:
  /// @note euqality has the lowest precedence
  auto expression() -> expr_ptr_t;
  /// @note equality has the second lowest precedence;
  ///			comparison generates equality.
  auto equality() -> expr_ptr_t;
  /// @note dittos
  auto comparison() -> expr_ptr_t;
  /// @note dittos
  auto term() -> expr_ptr_t;
  // /// @note dittos
  auto factor() -> expr_ptr_t;
  // /// @note dittos
  auto unary() -> expr_ptr_t;
  // /// @note dittos
  auto call() -> expr_ptr_t { TODO("implement call"); }
  // /// @note dittos
  auto primary() -> expr_ptr_t;

private:
  template <typename... Args>
    requires(std::is_enum_v<std::common_type_t<Args...>>)
  bool inspect(Args &&...args);
  bool is_at_end(const size_type offset = 0) const;
  auto get(size_type offset = 1) -> token_t;
  /// @note i dont want to write 4 different constness-related overloads. so
  /// just use deducing this.
  auto peek(this auto &&self, const ssize_type offset = 0) -> decltype(auto) {
    if (self.is_at_end(offset))
      return self.tokens.back();
    return self.tokens[self.current + offset];
  }

private:
  token_views_t tokens;
  size_type current = 0;
  expr_ptr_t expr_head = nullptr;
};
template <typename... Args>
  requires(std::is_enum_v<std::common_type_t<Args...>>)
bool parser::inspect(Args &&...args) {
  if (is_at_end())
    return false;
  return (... || (peek().type.type == args));
}
} // namespace net::ancillarycat::loxograph
