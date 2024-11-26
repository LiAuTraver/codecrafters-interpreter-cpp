#pragma once

#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>
#include <variant>
#include <iostream>
#include <source_location>
#include <type_traits>
#include <utility>


#include "config.hpp"
#include "Expr.hpp"
#include "parse_error.hpp"
#include "status.hpp"
#include "Token.hpp"
#include "TokenType.hpp"

namespace net::ancillarycat::loxograph {
class LOXOGRAPH_API parser {
public:
  using token_t = Token;
  using token_views_t = std::span<token_t>;
  using token_type_t = Token::token_type;
  using string_type = Token::string_type;
  using string_view_type = token_t::string_view_type;
  using size_type = token_views_t::size_type;
  using ssize_type = decltype(std::ssize(std::declval<token_views_t>()));
  using expr_t = expression::Expr;
  using expr_ptr_t = std::shared_ptr<expr_t>;
  using enum token_type_t::type_t;

public:
  parser() = default;
  parser &set_views(const token_views_t & = {});

public:
  /// @brief main entry point for parsing.
  /// @note Expression needs to be shared; especially for variables.
  ///  `(a + b) * (a + b)`. `(a + b)` is shared. `std::unique_ptr` may
  /// bring redundancy.
  auto parse() -> utils::Status;
  auto get_expr() const -> expr_ptr_t;

private:
  /// @brief euqality has the lowest precedence
  auto expression() -> expr_ptr_t;
  /// @brief equality has the second lowest precedence;
  ///			comparison generates equality.
  auto equality() -> expr_ptr_t;
  auto comparison() -> expr_ptr_t;
  auto term() -> expr_ptr_t;
  auto factor() -> expr_ptr_t;
  auto unary() -> expr_ptr_t;
  auto call() /* -> expr_ptr_t */ { TODO("implement call"); }
  auto primary() -> expr_ptr_t;
  /// FIXME: 1. do not use exceptions for control flow(possiblly)
  /// <br>
  /// FIXME: 2. add a field to the parser to track the error, not returns a shared_ptr
  auto recovery_parse(const parse_error &) -> expr_ptr_t;

private:
  template <typename... Args>
    requires(std::is_enum_v<std::common_type_t<Args...>>)
  bool inspect(Args &&...);
  /// @brief check if the current token is at(or past) the end of the token
  bool is_at_end(size_type = 0) const;
  auto get(size_type = 1) -> token_t;
  /// @brief get the current token(or the token at the offset) without advancing
  /// the cursor
  /// @param self the parser object
  /// @param offset the offset from the current token(optional, default to 0)
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
  bool is_in_panic = false;
};
template <typename... Args>
  requires(std::is_enum_v<std::common_type_t<Args...>>)
bool parser::inspect(Args &&...args) {
  if (is_at_end())
    return false;
  return (... || (peek().type.type == args));
}
} // namespace net::ancillarycat::loxograph
