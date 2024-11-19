#include "Token.hpp"
#include <any>
#include <concepts>
#include <cstdint>
#include <format>
#include <lex_error.hpp>
#include <optional>
#include <source_location>
#include <string>
#include <string_view>
#include <typeinfo>
#include <unordered_map>
#include <variant>
#include "config.hpp"
namespace net::ancillarycat::loxograph {
template <typename Ty>
inline auto Token::cast_literal() const
    -> std::optional<decltype(std::any_cast<Ty>(literal))>
  requires std::default_initializable<Ty> &&
           std::formattable<Ty, string_t::value_type>
{
  try {
    dbg(info, "literal type: {}, value: {}", literal.type().name(),
        std::any_cast<Ty>(literal));
    return std::any_cast<Ty>(literal);
  } catch (std::bad_any_cast &e) {
    dbg(error, "bad any cast: {}", e.what());
    return std::nullopt;
  }
}
template <typename Ty>
inline auto Token::cast_literal() const
    -> std::optional<decltype(std::any_cast<Ty>(literal))>
  requires std::default_initializable<Ty> &&
               (!std::formattable<Ty, string_t::value_type>) &&
               requires(Ty t) {
                 { t.to_string() } -> std::convertible_to<string_t>;
               }
{
  try {
    dbg(info, "literal type: {}, value: {}", literal.type().name(),
        std::any_cast<Ty>(literal).to_string());
    return std::any_cast<Ty>(literal);
  } catch (std::bad_any_cast &e) {
    dbg(error, "bad any cast: {}", e.what());
    return std::nullopt;
  }
}
Token::string_t Token::to_string() const {
  using namespace std::string_literals;
  using enum TokenType::type_t;
  auto type_sv = ""sv;
  auto lexeme_sv = ""sv;
  auto literal_sv = ""sv;
  if (!literal.has_value())
    literal_sv = "null"sv;
  switch (type.type) {
  case kMonostate:
    type_sv = "MONOSTATE"sv;
    lexeme_sv = "null"sv;
    literal_sv = "null"sv;
    break;
  case kLeftParen:
    type_sv = "LEFT_PAREN"sv;
    lexeme_sv = "("sv;
    literal_sv = "null"sv;
    break;
  case kRightParen:
    type_sv = "RIGHT_PAREN"sv;
    lexeme_sv = ")"sv;
    literal_sv = "null"sv;
    break;
  case kLeftBrace:
    type_sv = "LEFT_BRACE"sv;
    lexeme_sv = "{"sv;
    literal_sv = "null"sv;
    break;
  case kRightBrace:
    type_sv = "RIGHT_BRACE"sv;
    lexeme_sv = "}"sv;
    literal_sv = "null"sv;
    break;
  case kComma:
    type_sv = "COMMA"sv;
    lexeme_sv = ","sv;
    literal_sv = "null"sv;
    break;
  case kDot:
    type_sv = "DOT"sv;
    lexeme_sv = "."sv;
    literal_sv = "null"sv;
    break;
  case kMinus:
    type_sv = "MINUS"sv;
    lexeme_sv = "-"sv;
    literal_sv = "null"sv;
    break;
  case kPlus:
    type_sv = "PLUS"sv;
    lexeme_sv = "+"sv;
    literal_sv = "null"sv;
    break;
  case kSemicolon:
    type_sv = "SEMICOLON"sv;
    lexeme_sv = ";"sv;
    literal_sv = "null"sv;
    break;
  case kSlash:
    type_sv = "SLASH"sv;
    lexeme_sv = "/"sv;
    literal_sv = "null"sv;
    break;
  case kStar:
    type_sv = "STAR"sv;
    lexeme_sv = "*"sv;
    literal_sv = "null"sv;
    break;
  case kBang:
    type_sv = "BANG"sv;
    lexeme_sv = "!"sv;
    literal_sv = "null"sv;
    break;
  case kBangEqual:
    type_sv = "BANG_EQUAL"sv;
    lexeme_sv = "!="sv;
    literal_sv = "null"sv;
    break;
  case kEqual:
    type_sv = "EQUAL"sv;
    lexeme_sv = "="sv;
    literal_sv = "null"sv;
    break;
  case kEqualEqual:
    type_sv = "EQUAL_EQUAL"sv;
    lexeme_sv = "=="sv;
    literal_sv = "null"sv;
    break;
  case kGreater:
    type_sv = "GREATER"sv;
    lexeme_sv = ">"sv;
    literal_sv = "null"sv;
    break;
  case kGreaterEqual:
    type_sv = "GREATER_EQUAL"sv;
    lexeme_sv = ">="sv;
    literal_sv = "null"sv;
    break;
  case kLess:
    type_sv = "LESS"sv;
    lexeme_sv = "<"sv;
    literal_sv = "null"sv;
    break;
  case kLessEqual:
    type_sv = "LESS_EQUAL"sv;
    lexeme_sv = "<="sv;
    literal_sv = "null"sv;
    break;
  case kIdentifier:
    type_sv = "IDENTIFIER"sv;
    lexeme_sv = lexeme;
    literal_sv = "null"sv;
    break;
  case kString:
    type_sv = "STRING"sv;
    lexeme_sv = lexeme;
    contract_assert(lexeme_sv.front() == '"' && lexeme_sv.back() == '"');
    literal_sv =
        cast_literal<string_view_t>().value_or("<failed to access data>");
    contract_assert(lexeme_sv.substr(1, lexeme_sv.size() - 2), literal_sv);
    break;
  case kNumber:
    break;
  case kAnd:
    break;
  case kClass:
    break;
  case kElse:
    break;
  case kFalse:
    break;
  case kFun:
    break;
  case kFor:
    break;
  case kIf:
    break;
  case kNil:
    break;
  case kOr:
    break;
  case kPrint:
    break;
  case kReturn:
    break;
  case kSuper:
    break;
  case kThis:
    break;
  case kTrue:
    break;
  case kVar:
    type_sv = "VAR"sv;
    lexeme_sv = "var"sv;
    literal_sv = "null"sv;
  case kWhile:
    break;
  case kEndOfFile:
    type_sv = "EOF"sv;
    lexeme_sv = ""sv;
    literal_sv = "null"sv;
    break;
  case kLexError:
    /// @note message is different from the other cases.
    return cast_literal<error_t>().value_or(lex_error{}).to_string(lexeme, line);
  default:
    break;
  }
  return string_t{type_sv} + " " + string_t{lexeme_sv} + " " +
         string_t{literal_sv};
}
} // namespace net::ancillarycat::loxograph