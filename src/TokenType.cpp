#if __has_include(<spdlog/spdlog.h>)
#include <spdlog/spdlog.h>
#endif

#include <string>
#include <string_view>

#include "config.hpp"
#include "loxo_fwd.hpp"
#include "TokenType.hpp"
namespace net::ancillarycat::loxograph {
TokenType::string_view_t TokenType::to_string_view() const noexcept {
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
  case kLexError:
    return "LEX_ERROR"sv;
  default:
    dbg(error, "Unknown token type: {}", (uint16_t)type);
    return "UNKNOWN"sv;
  }
}
TokenType::string_t TokenType::to_string() const noexcept {
  return string_t{to_string_view()};
}
} // namespace net::ancillarycat::loxograph