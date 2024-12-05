#pragma once

#include <iostream>
#include <memory>
#include <source_location>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include "config.hpp"
#include "parse_error.hpp"
#include "statement.hpp"
#include "status.hpp"
#include "Token.hpp"
#include "TokenType.hpp"

namespace net::ancillarycat::loxo {
class LOXO_API parser {
public:
  enum ParsePolicy {
    kStatement = 0,
    kExpression = 1,
  };

public:
  using token_t = Token;
  using token_views_t = std::span<token_t>;
  using token_type_t = token_t::token_type;
  using string_type = token_t::string_type;
  using string_view_type = token_t::string_view_type;
  using size_type = token_views_t::size_type;
  using ssize_type = decltype(std::ssize(std::declval<token_views_t>()));
  using expr_t = expression::Expr;
  using expr_ptr_t = std::shared_ptr<expr_t>;
  using stmt_t = statement::Stmt;
  using stmt_ptr_t = std::shared_ptr<stmt_t>;
  using stmt_ptrs_t = std::vector<stmt_ptr_t>;
  using enum token_type_t::type_t;

public:
  parser() = default;
  parser &set_views(token_views_t = {});
  /// @brief main entry point for parsing.
  /// @note Expression needs to be shared; especially for variables.
  ///  `(a + b) * (a + b)`. `(a + b)` is shared. `std::unique_ptr` may
  /// bring redundancy.
  auto parse(const ParsePolicy &) -> utils::Status;
  auto get_statements() const -> stmt_ptrs_t &;
  auto get_expression() const -> expr_ptr_t &;

private:
  auto next_expression() -> expr_ptr_t;
  auto assignment() -> expr_ptr_t;
  auto logical_or() -> expr_ptr_t;
  auto logical_and() -> expr_ptr_t;
  auto equality() -> expr_ptr_t;
  auto comparison() -> expr_ptr_t;
  auto term() -> expr_ptr_t;
  auto factor() -> expr_ptr_t;
  auto unary() -> expr_ptr_t;
  auto call() -> expr_ptr_t;
  auto primary() -> expr_ptr_t;

private:
  auto get_args() -> std::vector<expr_ptr_t>;
  auto get_params() -> std::vector<token_t>;
  auto get_stmts() -> stmt_ptrs_t;

private:
  auto next_declaration() -> stmt_ptr_t;
  auto next_statement() -> stmt_ptr_t;
  auto expr_stmt() -> stmt_ptr_t;
  auto print_stmt() -> stmt_ptr_t;
  auto if_stmt() -> stmt_ptr_t;
  auto block_stmt() -> stmt_ptr_t;
  auto while_stmt() -> stmt_ptr_t;
  auto for_stmt() -> stmt_ptr_t;
  auto var_decl() -> stmt_ptr_t;
  auto function_decl() -> stmt_ptr_t;

  /// FIXME: 1. do not use exceptions for control flow(possiblly)
  /// <br>
  /// FIXME: 2. add a field to the parser to track the error, not returns a
  ///         shared_ptr
  auto synchronize(const parse_error &) -> expr_ptr_t;

private:
  /// @remark used in @link while_stmt @endlink and @link if_stmt @endlink
  auto get_condition() -> expr_ptr_t;

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
    return *(self.cursor + offset); // ref to the token
  }

private:
  token_views_t tokens = {};
  token_views_t::iterator cursor{};
  mutable expr_ptr_t expr_head = nullptr;
  mutable stmt_ptrs_t stmts = {};
  // bool is_in_panic = false;
private:
  friend LOXO_API void delete_parser_fwd(parser *);
};
template <typename... Args>
  requires(std::is_enum_v<std::common_type_t<Args...>>)
bool parser::inspect(Args &&...args) {
  if (is_at_end())
    return false;
  return (... || (peek().type.type == args));
}
} // namespace net::ancillarycat::loxo
