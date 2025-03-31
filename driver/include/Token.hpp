#ifndef AC_LOXO_TOKEN_HPP
#define AC_LOXO_TOKEN_HPP

#include <concepts>
#include <cstdint>
#include <limits>
#include <numeric>
#include <string>
#include <type_traits>

#include "accat/auxilia/details/Monostate.hpp"
#include "accat/auxilia/details/Variant.hpp"
#include "accat/auxilia/details/format.hpp"
#include "details/loxo_fwd.hpp"

#ifdef AC_LOXO_DETAILS_TOKENTYPE_HPP
#  error                                                                       \
      "please do not include TokenType.hpp in other files; include Token.hpp instead"
#endif
#include "details/TokenType.inl"

namespace accat::loxo {
class LOXO_API Token : public auxilia::Printable {
public:
  using token_type = TokenType;
  using error_t = lex_error;
  using string_view_type = auxilia::string_view;
  using literal_type = auxilia::
      Variant<auxilia::Monostate, string_view_type, long long, long double, bool>;


public:
  Token() = default;
  Token(const token_type type,
        string_view_type lexeme,
        const literal_type &literal = {},
        uint_least32_t line = std::numeric_limits<
            std::underlying_type_t<enum token_type::type_t>>::signaling_NaN())
      : type(type), lexeme(lexeme), literal(literal), line(line) {}
public:
  string_type number_to_string(auxilia::FormatPolicy policy) const;
  constexpr auto is_type(const token_type &type) const noexcept -> bool {
    return this->type == type;
  }

public:
  auto to_string(const auxilia::FormatPolicy & =
                     auxilia::FormatPolicy::kDefault) const -> string_type;

public:
  /// @brief the type of the token
  token_type type{TokenType::kMonostate};
  /// @brief the lexeme. (the actual string)
  string_type lexeme;
  /// @brief the literal value of the token
  literal_type literal;
  /// @brief the line number where the token is found
  uint_least32_t line = std::numeric_limits<
      std::underlying_type_t<enum token_type::type_t>>::signaling_NaN();

private:
  friend auto format_as(const Token &token) -> Token::string_type {
    return token.to_string(auxilia::FormatPolicy::kDefault);
  }
};
} // namespace accat::loxo

#endif // AC_LOXO_TOKEN_HPP
