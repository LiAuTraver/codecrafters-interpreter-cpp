#pragma once
#include <any>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <variant>
#include "TokenType.hpp"
#include "config.hpp"
#include "loxo_fwd.hpp"
#if __has_include(<spdlog/spdlog.h>)
#include <spdlog/spdlog.h>
#endif
namespace net::ancillarycat::loxograph {
static const auto keywords = std::unordered_map<StringViewType, TokenType>{
    {"and"sv, {TokenType::kAnd}},       {"class"sv, {TokenType::kClass}},
    {"else"sv, {TokenType::kElse}},     {"false"sv, {TokenType::kFalse}},
    {"for"sv, {TokenType::kFor}},       {"fun"sv, {TokenType::kFun}},
    {"if"sv, {TokenType::kIf}},         {"nil"sv, {TokenType::kNil}},
    {"or"sv, {TokenType::kOr}},         {"print"sv, {TokenType::kPrint}},
    {"return"sv, {TokenType::kReturn}}, {"super"sv, {TokenType::kSuper}},
    {"this"sv, {TokenType::kThis}},     {"true"sv, {TokenType::kTrue}},
    {"var"sv, {TokenType::kVar}},       {"while"sv, {TokenType::kWhile}},
};
class Token {
public:
  using string_t = StringType;
  using string_view_t = StringViewType;
  using token_type = TokenType;
  using error_t = lex_error;

public:
  inline Token() : type(TokenType::kMonostate) {}
  inline explicit
  Token(token_type type, string_t lexeme = std::string{},
        std::any literal = std::any{},
        uint_least32_t line = std::numeric_limits<
            std::underlying_type_t<enum token_type::type_t>>::signaling_NaN())
      : type(type), lexeme(lexeme), literal(literal), line(line) {}
  inline explicit
  Token(token_type type, string_view_t lexeme = std::string_view{},
        std::any literal = std::any{},
        uint_least32_t line = std::numeric_limits<
            std::underlying_type_t<enum token_type::type_t>>::signaling_NaN())
      : type(type), lexeme(lexeme), literal(literal), line(line) {}

public:
  nodiscard_msg(string_t) string_t to_string() const;

public:
  token_type type = TokenType::kMonostate;
  string_t lexeme;
  std::any literal = std::any();
  uint_least32_t line;

private:
  template <typename Ty>
  inline auto cast_literal() const
      -> std::optional<decltype(std::any_cast<Ty>(literal))>
    requires std::default_initializable<Ty> &&
             std::formattable<Ty, string_t::value_type>;
  template <typename Ty>
  inline auto cast_literal() const
      -> std::optional<decltype(std::any_cast<Ty>(literal))>
    requires std::default_initializable<Ty> &&
                 (!std::formattable<Ty, string_t::value_type>) && requires(Ty t) {
                   { t.to_string() } -> std::convertible_to<string_t>;
                 };
};
} // namespace net::ancillarycat::loxograph
