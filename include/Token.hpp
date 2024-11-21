#pragma once
#include <any>

#include "config.hpp"
#include "loxo_fwd.hpp"
#include "TokenType.hpp"

namespace net::ancillarycat::loxograph {
class LOXOGRAPH_API Token {
public:
  using string_type = TokenType::string_t;
  using string_view_type = TokenType::string_view_t;
  using token_type = TokenType;
  using error_t = lex_error;

public:
  Token() : type(TokenType::kMonostate) {}
  explicit Token(
      token_type type, string_type lexeme = std::string{},
      std::any literal = std::any{},
      uint_least32_t line = std::numeric_limits<
          std::underlying_type_t<enum token_type::type_t>>::signaling_NaN())
      : type(type), lexeme(std::move(lexeme)), literal(std::move(literal)),
        line(line) {}
  explicit Token(
      token_type type, string_view_type lexeme = std::string_view{},
      std::any literal = std::any{},
      uint_least32_t line = std::numeric_limits<
          std::underlying_type_t<enum token_type::type_t>>::signaling_NaN())
      : type(type), lexeme(lexeme), literal(std::move(literal)), line(line) {}

public:
  nodiscard_msg(string_t) string_type to_string() const;

public:
  token_type type = TokenType::kMonostate;
  string_type lexeme = string_type();
  std::any literal = std::any();
  uint_least32_t line = std::numeric_limits<
      std::underlying_type_t<enum token_type::type_t>>::signaling_NaN();
  friend auto format_as(const Token &) -> Token::string_type;

private:
  template <typename Ty>
  inline auto cast_literal() const
      -> decltype(auto) // <- mix const/no-const pointer would result in a
                        // compile error
    requires std::default_initializable<Ty>;
  template <typename Ty>
    requires std::is_arithmetic_v<std::remove_cvref_t<Ty>>
  inline bool is_integer(Ty &&value) const noexcept;
};
} // namespace net::ancillarycat::loxograph

template <>
struct std::formatter<net::ancillarycat::loxograph::Token>
    : std::formatter<net::ancillarycat::loxograph::Token::string_type> {
  auto format(const net::ancillarycat::loxograph::Token &,
              std::format_context &ctx) const -> decltype(ctx.out());
};
