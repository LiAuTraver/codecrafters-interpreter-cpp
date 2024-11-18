#pragma once
#include <any>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <variant>
#include "config.hpp"
#include "loxo_fwd.hpp"
#if __has_include(<spdlog/spdlog.h>)
#include "spdlog/spdlog.h"
#endif
namespace net::ancillarycat::loxograph {
class TokenType {
public:
  enum type : uint16_t {
    kMonostate = 0,
    // Single-character tokens.
    kLeftParen,
    kRightParen,
    kLeftBrace,
    kRightBrace,
    kComma,
    kDot,
    kMinus,
    kPlus,
    kSemicolon,
    kSlash,
    kStar,

    // One or two character tokens.
    kBang,
    kBangEqual,
    kEqual,
    kEqualEqual,
    kGreater,
    kGreaterEqual,
    kLess,
    kLessEqual,

    // Literals.
    kIdentifier,
    kString,
    kNumber,

    // Keywords.
    kAnd,
    kClass,
    kElse,
    kFalse,
    kFun,
    kFor,
    kIf,
    kNil,
    kOr,
    kPrint,
    kReturn,
    kSuper,
    kThis,
    kTrue,
    kVar,
    kWhile,

    kEndOfFile,
  } type;

public:
  using string_t = StringType;
  using string_view_t = StringViewType;

public:
  /// @note no-explicit
  inline TokenType(enum type type = kMonostate) : type(type) {}

public:
  string_view_t to_string_view() const noexcept;
  string_t to_string() const noexcept;
};
using namespace std::string_view_literals;
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

public:
  inline explicit
  Token(token_type type = TokenType::kMonostate,
        string_t lexeme = std::string{}, std::any literal = std::any{},
        uint_least32_t line = std::numeric_limits<
            std::underlying_type_t<enum token_type::type>>::signaling_NaN())
      : type(type), lexeme(lexeme), literal(literal), line(line) {}
  inline explicit
  Token(token_type type = TokenType::kMonostate,
        string_view_t lexeme = std::string_view{},
        std::any literal = std::any{},
        uint_least32_t line = std::numeric_limits<
            std::underlying_type_t<enum token_type::type>>::signaling_NaN())
      : type(type), lexeme(lexeme), literal(literal), line(line) {}

public:
  string_t to_string() const ;
  string_view_t to_string_view() const ;

public:
public:
  token_type type;
  string_t lexeme;
  std::any literal;
  uint_least32_t line;

private:
};
static constexpr auto tolerable_chars = "_$@`"sv;
static constexpr auto whitespace_chars = " \t\r"sv;
static constexpr auto newline_chars = "\n\v\f"sv;
inline TokenType::string_view_t TokenType::to_string_view() const noexcept {
  using namespace std::string_view_literals;
  switch (type) {
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
  default:
    dbg(error, "Unknown token type: {}", (uint16_t)type);
    return "UNKNOWN"sv;
    break;
  }
}
inline TokenType::string_t TokenType::to_string() const noexcept {
  using namespace std::string_literals;
  switch (type) {
  case kMonostate:
    return "MONOSTATE"s;
  case kLeftParen:
    return "LEFT_PAREN"s;
  case kRightParen:
    return "RIGHT_PAREN"s;
  case kLeftBrace:
    return "LEFT_BRACE"s;
  case kRightBrace:
    return "RIGHT_BRACE"s;
  case kComma:
    return "COMMA"s;
  case kDot:
    return "DOT"s;
  case kMinus:
    return "MINUS"s;
  case kPlus:
    return "PLUS"s;
  case kSemicolon:
    return "SEMICOLON"s;
  case kSlash:
    return "SLASH"s;
  case kStar:
    return "STAR"s;
  case kBang:
    return "BANG"s;
  case kBangEqual:
    return "BANG_EQUAL"s;
  case kEqual:
    return "EQUAL"s;
  case kEqualEqual:
    return "EQUAL_EQUAL"s;
  case kGreater:
    return "GREATER"s;
  case kGreaterEqual:
    return "GREATER_EQUAL"s;
  case kLess:
    return "LESS"s;
  case kLessEqual:
    return "LESS_EQUAL"s;
  case kIdentifier:
    return "IDENTIFIER"s;
  case kString:
    return "STRING"s;
  case kNumber:
    return "NUMBER"s;
  case kAnd:
    return "AND"s;
  case kClass:
    return "CLASS"s;
  case kElse:
    return "ELSE"s;
  case kFalse:
    return "FALSE"s;
  case kFun:
    return "FUN"s;
  case kFor:
    return "FOR"s;
  case kIf:
    return "IF"s;
  case kNil:
    return "NIL"s;
  case kOr:
    return "OR"s;
  case kPrint:
    return "PRINT"s;
  case kReturn:
    return "RETURN"s;
  case kSuper:
    return "SUPER"s;
  case kThis:
    return "THIS"s;
  case kTrue:
    return "TRUE"s;
  case kVar:
    return "VAR"s;
  case kWhile:
    return "WHILE"s;
  case kEndOfFile:
    return "EOF"s;
  default:
    dbg(error, "Unknown token type: {}", (uint16_t)type);
    return "UNKNOWN"s;
  }
}
inline Token::string_t Token::to_string() const {
        // ENUM LEXEME LITERAL(or null)
        return std::format("{} {} {}", type.to_string(), lexeme,
                         literal.has_value() ? "literal" : "null");
}
} // namespace net::ancillarycat::loxograph
template <>
struct std::formatter<net::ancillarycat::loxograph::Token>
    : std::formatter<net::ancillarycat::loxograph::Token::string_t> {
  auto format(net::ancillarycat::loxograph::Token token,
              std::format_context &ctx) -> decltype(ctx.out()) {
    // return std::format_to(ctx.out(),
    //                       "[type = {0}, lexeme = {1}, literal_type = {2},
    //                       line = {3}]", "todo", token.lexeme.empty() ?
    //                       "<empty>" :token.lexeme,
    //                       token.literal.type().name(), token.line);
  }
};

// template <>
// struct fmt::formatter<net::ancillarycat::loxograph::Token>
//     : formatter<net::ancillarycat::loxograph::Token::string_t> {
//   auto fmt::format(net::ancillarycat::loxograph::Token token,
//                    format_context &ctx) -> decltype(ctx.out()) {
//     return fmt::format_to(
//         ctx.out(), "[type = {0}, lexeme = {1}, literal_type = {2}, line =
//         {3}]", magic_enum::enum_name(token.type), token.lexeme.empty() ?
//         "<empty>" : token.lexeme, token.literal.type().name(), token.line);
//   }
// };
