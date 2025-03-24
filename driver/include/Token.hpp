#ifndef AC_LOXO_TOKEN_HPP
#define AC_LOXO_TOKEN_HPP
#include <any>
#include <concepts>
#include <cstdint>
#include <limits>
#include <numeric>
#include <string>

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

public:
  Token() = default;
  explicit Token(
      const token_type &type,
      string_type lexeme = std::string{},
      std::any literal = std::any{},
      uint_least32_t line = std::numeric_limits<
          std::underlying_type_t<enum token_type::type_t>>::signaling_NaN());
  explicit Token(
      const token_type &type,
      string_view_type lexeme = std::string_view{},
      std::any literal = std::any{},
      uint_least32_t line = std::numeric_limits<
          std::underlying_type_t<enum token_type::type_t>>::signaling_NaN());

public:
  string_type number_to_string(auxilia::FormatPolicy policy) const;
  constexpr auto is_type(const token_type &type) const noexcept -> bool {
    return this->type == type;
  }

public:
  auto to_string(const auxilia::FormatPolicy & = auxilia::FormatPolicy::kDefault) const
      -> string_type;

public:
  /// @brief the type of the token
  token_type type{TokenType::kMonostate};
  /// @brief the lexeme. (the actual string)
  string_type lexeme = string_type();
  /// @brief the literal value of the token
  dbg_only(mutable)
  std::any literal = std::any();
  /// @brief the line number where the token is found
  uint_least32_t line = std::numeric_limits<
      std::underlying_type_t<enum token_type::type_t>>::signaling_NaN();

private:
  friend auto format_as(const Token &) -> Token::string_type;
  template <typename Ty>
  inline auto cast_literal() const
      -> decltype(auto) // <- mix const/no-const pointer would
                        // result in a compile error
    requires std::default_initializable<Ty>;
};
} // namespace accat::loxo

#endif // AC_LOXO_TOKEN_HPP
