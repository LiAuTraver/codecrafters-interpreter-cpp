#include "std.hh"

#include "config.hpp"
#include "lex_error.hpp"
#include "loxo_fwd.hpp"
#include "Token.hpp"

namespace net::ancillarycat::loxograph {
// clang-format off
/*!
   @brief cast the literal to the specified type and also log the value in debug mode.
   @note clang-cl.exe does not fully support `try` and `catch` blocks but
   `any_cast` will throw an exception if the cast fails. For more information, see
   <a href="https://clang.llvm.org/docs/MSVCCompatibility.html#asynchronous-exceptions">Asynchronous Exceptions</a>,
   <a href="https://stackoverflow.com/questions/7049502/c-try-and-try-catch-finally">try-catch-finally</a>,
   and <a href="https://learn.microsoft.com/en-us/cpp/cpp/try-except-statement">try-except-statement</a>.
 */
// clang-format on
template <typename Ty>
inline auto Token::cast_literal() const -> decltype(auto)
  requires std::default_initializable<Ty>
{
  const auto ptr = std::any_cast<Ty>(&literal);
  if (ptr) {
    dbg(info, "literal type: {}, value: {}", typeid(Ty).name(), *ptr);
  } else {
    dbg_block({
      dbg(error, "bad any cast: {}", LOXOGRAPH_STACKTRACE);
      dbg(info,
          "Expect type: {}, actual type: {}",
          typeid(Ty).name(),
          literal.type().name());
      dbg(warn,
          "\033[033mNote: this lexer treat all number as long double; meybe "
          "you "
          "accidentally passed an integer?\033[0m");
      contract_assert(0);
    });
  }
  return ptr;
}
template <typename Ty>
  requires std::is_arithmetic_v<std::remove_cvref_t<Ty>>
bool Token::is_integer(Ty &&value) const noexcept {
  return std::trunc(std::forward<Ty>(value)) == value;
}
Token::string_type Token::number_to_string(const FormatPolicy policy) const {
  if (auto ptr = cast_literal<long double>()) {
    // 42 -> 42.0
    if (is_integer(*ptr)) {
      if (policy == kDefault)
        return utils::format("NUMBER {} {:.1f}", lexeme, *ptr);
      else if (policy == kTokenOnly)
        return utils::format("{:.1f}", *ptr);
      else {
        dbg(critical, "unreachable code reached: {}", LOXOGRAPH_STACKTRACE);
        contract_assert(false);
        std::unreachable();
      }
    }
    //  leave as is
    if (policy == kDefault)
      return utils::format("NUMBER {} {}", lexeme, *ptr);
    else if (policy == kTokenOnly)
      return utils::format("{}", *ptr);
    else {
      dbg(critical, "unreachable code reached: {}", LOXOGRAPH_STACKTRACE);
      contract_assert(false);
      std::unreachable();
    }
  } else {
    dbg_block(literal = nullptr;);
    if (policy == kDefault)
      return utils::format("NUMBER {} {}", lexeme, "<failed to access data>");
    else if (policy == kTokenOnly)
      return utils::format("{}", "<failed to access data>");
    else {
      dbg(critical, "unreachable code reached: {}", LOXOGRAPH_STACKTRACE);
      contract_assert(false);
      std::unreachable();
    }
  }
}
Token::string_type Token::to_string(const FormatPolicy policy) const {
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
    // ptr = cast_literal<string_view_type>();
    // literal_sv =
    if (auto ptr = cast_literal<string_view_type>())
      literal_sv = *ptr;
    else
      literal_sv = "<failed to access data>"sv;
    contract_assert(lexeme_sv.substr(1, lexeme_sv.size() - 2), literal_sv);
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
      // /// @note message is different from the other cases.
      if (auto ptr = cast_literal<error_t>())
        return ptr->to_string(lexeme, line);
      else
        return utils::format(
            "[line {}] Error: {}", line, "<failed to access data>");
    } else {
      // do nothing
      return ""s;
    }
  default:
    dbg(critical, "unreachable code reached: {}", LOXOGRAPH_STACKTRACE);
    contract_assert(false);
    break;
  }
  if (policy == kDefault) {
    /// @note DON'T use `.data()` since it's not null-terminated and will the
    /// string will last till the end
    return utils::format("{} {} {}", type_sv, lexeme_sv, literal_sv);
  } else if (policy == kTokenOnly) {
    // for ast print.
    return utils::format("{}", lexeme_sv);
  } else {
    dbg(critical, "unreachable code reached: {}", LOXOGRAPH_STACKTRACE);
    contract_assert(false);
    std::unreachable();
  }
}
auto format_as(const Token &token) -> Token::string_type {
  return token.to_string();
}
} // namespace net::ancillarycat::loxograph
