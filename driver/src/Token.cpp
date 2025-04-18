
#include <concepts>
#include <limits>
#include <numeric>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <cmath>
#include <source_location>

#include "details/lox_fwd.hpp"

#include "details/lex_error.hpp"
#include "Token.hpp"

namespace accat::lox {
using enum auxilia::FormatPolicy;
Token::string_type
Token::number_to_string(const auxilia::FormatPolicy policy) const {
  if (auto ptr = literal.get_if<long double>()) {
    // 42 -> 42.0
    if (auxilia::is_integer(*ptr)) {
      if (policy == kDefault)
        return auxilia::format("NUMBER {} {:.1f}", lexeme, *ptr);
      if (policy == kDetailed)
        return auxilia::format("{:.1f}", *ptr);
      dbg_break
    }
    // leave as is
    if (policy == kDefault)
      return auxilia::format("NUMBER {} {}", lexeme, *ptr);
    if (policy == kDetailed)
      return auxilia::format("{}", *ptr);
    dbg_break
  } else {
    if (policy == kDefault)
      return auxilia::format("NUMBER {} {}", lexeme, "<failed to access data>");
    if (policy == kDetailed)
      return auxilia::format("{}", "<failed to access data>");
    dbg_break
  }
  TODO(...)
}
Token::string_type
Token::to_string(const auxilia::FormatPolicy &policy) const {
  using namespace std::string_literals;
  using enum TokenType::type_t;
  auto type_sv = ""sv;
  auto lexeme_sv = ""sv;
  auto literal_sv = ""sv;
  if (literal.empty())
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
    contract_assert(lexeme_sv.front() == '"' && lexeme_sv.back() == '"',
                    "string literal should be enclosed in double quotes")
    if (policy == auxilia::FormatPolicy::kDetailed) {
      // codecrafter's string lit pase output does not need `"`, so remove them
      lexeme_sv = lexeme_sv.substr(1, lexeme_sv.size() - 2);
    }
    if (auto ptr = literal.get_if<string_view_type>()) 
      literal_sv = *ptr;
    else
      literal_sv = "<failed to access data>"sv;
    if (policy == kDefault)
      contract_assert(lexeme_sv.substr(1, lexeme_sv.size() - 2)== literal_sv,
                      "string literal should be the same as the literal")
    break;
  case kNumber:
    return number_to_string(policy);
  case kAnd:
    type_sv = "AND"sv;
    lexeme_sv = "and"sv;
    literal_sv = "null"sv;
    break;
  case kClass:
    type_sv = "CLASS"sv;
    lexeme_sv = "class"sv;
    literal_sv = "null"sv;
    break;
  case kElse:
    type_sv = "ELSE"sv;
    lexeme_sv = "else"sv;
    literal_sv = "null"sv;
    break;
  case kFalse:
    type_sv = "FALSE"sv;
    lexeme_sv = "false"sv;
    literal_sv = "null"sv;
    break;
  case kFun:
    type_sv = "FUN"sv;
    lexeme_sv = "fun"sv;
    literal_sv = "null"sv;
    break;
  case kFor:
    type_sv = "FOR"sv;
    lexeme_sv = "for"sv;
    literal_sv = "null"sv;
    break;
  case kIf:
    type_sv = "IF"sv;
    lexeme_sv = "if"sv;
    literal_sv = "null"sv;
    break;
  case kNil:
    type_sv = "NIL"sv;
    lexeme_sv = "nil"sv;
    literal_sv = "null"sv;
    break;
  case kOr:
    type_sv = "OR"sv;
    lexeme_sv = "or"sv;
    literal_sv = "null"sv;
    break;
  case kPrint:
    type_sv = "PRINT"sv;
    lexeme_sv = "print"sv;
    literal_sv = "null"sv;
    break;
  case kReturn:
    type_sv = "RETURN"sv;
    lexeme_sv = "return"sv;
    literal_sv = "null"sv;
    break;
  case kSuper:
    type_sv = "SUPER"sv;
    lexeme_sv = "super"sv;
    literal_sv = "null"sv;
    break;
  case kThis:
    type_sv = "THIS"sv;
    lexeme_sv = "this"sv;
    literal_sv = "null"sv;
    break;
  case kTrue:
    type_sv = "TRUE"sv;
    lexeme_sv = "true"sv;
    literal_sv = "null"sv;
    break;
  case kVar:
    type_sv = "VAR"sv;
    lexeme_sv = "var"sv;
    literal_sv = "null"sv;
    break;
  case kWhile:
    type_sv = "WHILE"sv;
    lexeme_sv = "while"sv;
    literal_sv = "null"sv;
    break;
  case kEndOfFile:
    type_sv = "EOF"sv;
    lexeme_sv = ""sv;
    literal_sv = "null"sv;
    break;
  case kLexError:
    if (policy == kDefault) {
      /// @note message is different from the other cases.
      if (auto ptr = this->literal.get_if<error_t>())
        return ptr->to_string(lexeme, line);
      else
        return auxilia::format(
            "[line {}] Error: {}", line, "<failed to access data>");
    } else {
      // do nothing
      return ""s;
    }
  default:
    dbg_break
    break;
  }
  if (policy == kDefault) {
    /// @note DON'T use `.data()` since it's not null-terminated and will the
    /// string will last till the end
    return auxilia::format("{} {} {}", type_sv, lexeme_sv, literal_sv);
  } else if (policy == kDetailed) {
    // for ast print.
    return auxilia::format("{}", lexeme_sv);
  }
  dbg_break
  return ""s;
}
} // namespace accat::lox
