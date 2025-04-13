/// @note no include guard.
#ifdef AC_LOX_DETAILS_TOKENTYPE_HPP
#  error                                                                       \
      "please do not include TokenType.hpp in other files; include Token.hpp instead"
#endif
#define AC_LOX_DETAILS_TOKENTYPE_HPP

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>

#include "details/lox_fwd.hpp"

namespace accat::lox {
/// @brief enhanced token type, more like rust's enum
/// @implements auxilia::Printable
/// @implements auxilia::Viewable
class AC_LOX_API TokenType : public auxilia::Printable,
                           public auxilia::Viewable {
public:
  enum type_t : uint16_t;
  using Viewable::string_view_type;

public:
  enum type_t : uint16_t {
    // clang-format off
    kMonostate = 0,
    // Single-character tokens.
    kLeftParen, kRightParen, kLeftBrace, kRightBrace, kComma, kDot, kMinus, kPlus, kSemicolon, kSlash, kStar,
    // One or two character tokens.
    kBang, kBangEqual, kEqual, kEqualEqual, kGreater, kGreaterEqual, kLess, kLessEqual,
    // Literals.
    kIdentifier, kString, kNumber,
    // Keywords.
    kAnd, kClass, kElse, kFalse, kFun, kFor, kIf, kNil, kOr, kPrint, kReturn, kSuper, kThis, kTrue, kVar, kWhile,
    // lex error.
    kLexError,
    // end of file.
    kEndOfFile,
    // clang-format on
  };

public:
  /// @note no-explicit
  TokenType(const enum type_t type = kMonostate)
      : type(type) {} // NOLINT(google-explicit-constructor)
  /// @brief for @link fmt::format @endlink
  friend auto format_as(const TokenType &) noexcept -> string_view_type;
  /// @note member function otherwise `ambiguous call` since the ctor is not
  /// explicit
  constexpr auto operator<=>(this auto &&self, const type_t &token_t) {
    return self.type <=> token_t;
  }
  friend constexpr auto operator==(const TokenType &wrapped_token_t,
                                   const type_t &token_t) {
    return wrapped_token_t.type == token_t;
  }
  friend constexpr auto operator<=>(const type_t &token_t,
                                    const TokenType &wrapped_token_t) {
    return token_t <=> wrapped_token_t.type;
  }
  friend constexpr auto operator==(const type_t &token_t,
                                   const TokenType &wrapped_token_t) {
    return token_t == wrapped_token_t.type;
  }
  friend constexpr auto operator<=>(const TokenType &lhs,
                                    const TokenType &rhs) {
    return lhs.type <=> rhs.type;
  }
  friend constexpr auto operator==(const TokenType &lhs, const TokenType &rhs) {
    return lhs.type == rhs.type;
  }
  friend constexpr auto operator!=(const TokenType &lhs, const TokenType &rhs) {
    return lhs.type != rhs.type;
  }

public:
  type_t type;

public:
  auto to_string_view(const auxilia::FormatPolicy &) const -> string_view_type;
  auto to_string(const auxilia::FormatPolicy &) const -> string_type;

};
inline static const auto keywords =
    std::unordered_map<TokenType::string_view_type, TokenType>{
        {"and"sv, {TokenType::kAnd}},
        {"class"sv, {TokenType::kClass}},
        {"else"sv, {TokenType::kElse}},
        {"false"sv, {TokenType::kFalse}},
        {"for"sv, {TokenType::kFor}},
        {"fun"sv, {TokenType::kFun}},
        {"if"sv, {TokenType::kIf}},
        {"nil"sv, {TokenType::kNil}},
        {"or"sv, {TokenType::kOr}},
        {"print"sv, {TokenType::kPrint}},
        {"return"sv, {TokenType::kReturn}},
        {"super"sv, {TokenType::kSuper}},
        {"this"sv, {TokenType::kThis}},
        {"true"sv, {TokenType::kTrue}},
        {"var"sv, {TokenType::kVar}},
        {"while"sv, {TokenType::kWhile}},
    };
inline TokenType::string_view_type
TokenType::to_string_view(const auxilia::FormatPolicy &) const {
  return string_view_type{format_as(*this)};
}
inline TokenType::string_type
TokenType::to_string(const auxilia::FormatPolicy &) const {
  return string_type{format_as(*this)};
}
inline auto format_as(const TokenType &t) noexcept
    -> TokenType::string_view_type {
  using enum TokenType::type_t;
  switch (t.type) {
  case kMonostate:
    return "MONOSTATE"sv;
  case kLeftParen:
    return "LEFT_PAREN"sv;
  case kRightParen:
    return "RIGHT_PAREN"sv;
  case kLeftBrace:
    return "LEFT_BRACE"sv;
  case kRightBrace:
    return "RIGHT_BRACE"sv;
  case kComma:
    return "COMMA"sv;
  case kDot:
    return "DOT"sv;
  case kMinus:
    return "MINUS"sv;
  case kPlus:
    return "PLUS"sv;
  case kSemicolon:
    return "SEMICOLON"sv;
  case kSlash:
    return "SLASH"sv;
  case kStar:
    return "STAR"sv;
  case kBang:
    return "BANG"sv;
  case kBangEqual:
    return "BANG_EQUAL"sv;
  case kEqual:
    return "EQUAL"sv;
  case kEqualEqual:
    return "EQUAL_EQUAL"sv;
  case kGreater:
    return "GREATER"sv;
  case kGreaterEqual:
    return "GREATER_EQUAL"sv;
  case kLess:
    return "LESS"sv;
  case kLessEqual:
    return "LESS_EQUAL"sv;
  case kIdentifier:
    return "IDENTIFIER"sv;
  case kString:
    return "STRING"sv;
  case kNumber:
    return "NUMBER"sv;
  case kAnd:
    return "AND"sv;
  case kClass:
    return "CLASS"sv;
  case kElse:
    return "ELSE"sv;
  case kFalse:
    return "FALSE"sv;
  case kFun:
    return "FUN"sv;
  case kFor:
    return "FOR"sv;
  case kIf:
    return "IF"sv;
  case kNil:
    return "NIL"sv;
  case kOr:
    return "OR"sv;
  case kPrint:
    return "PRINT"sv;
  case kReturn:
    return "RETURN"sv;
  case kSuper:
    return "SUPER"sv;
  case kThis:
    return "THIS"sv;
  case kTrue:
    return "TRUE"sv;
  case kVar:
    return "VAR"sv;
  case kWhile:
    return "WHILE"sv;
  case kEndOfFile:
    return "EOF"sv;
  case kLexError:
    return "LEX_ERROR"sv;
  default:
    dbg(error, "Unknown token type: {}", (uint16_t)t.type);
    return "UNKNOWN"sv;
  }
}
} // namespace accat::lox
