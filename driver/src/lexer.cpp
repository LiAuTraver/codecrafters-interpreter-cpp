#include <charconv>
#include <concepts>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <source_location>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "details/lox_fwd.hpp"

#include "details/lex_error.hpp"
#include "lexer.hpp"
#include "Token.hpp"

/// @namespace accat::lox
namespace accat::lox {
template <typename Predicate>
bool lexer::advance_if(Predicate &&predicate)
  requires std::invocable<Predicate, char_t> &&
           std::convertible_to<Predicate, bool>
{
  if (is_at_end() || !predicate(contents[cursor]))
    return false;
  cursor++;
  return true;
}
template <typename Num>
  requires std::is_arithmetic_v<Num>
auto to_number(std::string_view value) -> lexer::literal_type {
  Num number;
  const auto &[p, ec] =
      std::from_chars(value.data(), value.data() + value.size(), number);
  if (ec == std::errc())
    return number;
  dbg(error, "Unable to convert string to number: {}", value)
  dbg(error, "Error: {}", std::make_error_code(ec).message())
  return {};
}
lexer::lexer(lexer &&other) noexcept
    : head(std::exchange(other.head, 0)),
      cursor(std::exchange(other.cursor, 0)),
      contents(std::move(const_cast<string_type &>(other.contents))),
      lexeme_views(std::move(other.lexeme_views)),
      current_line(std::exchange(other.current_line, 1)),
      tokens(std::move(other.tokens)),
      error_count(std::exchange(other.error_count, 0)) {}
lexer &lexer::operator=(lexer &&other) noexcept {
  if (this == &other)
    return *this;
  head = std::exchange(other.head, 0);
  cursor = std::exchange(other.cursor, 0);
  const_cast<string_type &>(contents) =
      std::move(const_cast<string_type &>(other.contents));
  lexeme_views = std::move(other.lexeme_views);
  current_line = std::exchange(other.current_line, 1);
  tokens = std::move(other.tokens);
  error_count = std::exchange(other.error_count, 0);

  return *this;
}
lexer::status_t lexer::load(const path_type &filepath) const {
  if (not contents.empty())
    return auxilia::AlreadyExistsError("File already loaded");
  if (not std::filesystem::exists(filepath))
    return auxilia::NotFoundError("File does not exist: " + filepath.string());
  auto file = std::ifstream{filepath};
  if (not file)
    return auxilia::PermissionDeniedError("Unable to open file " +
                                          filepath.string());
  auto buffer = std::ostringstream{};
  buffer << file.rdbuf();
  const_cast<string_type &>(contents) = buffer.str();
  return {};
}
lexer::status_t lexer::load(const std::istream &ss) {
  if (not contents.empty())
    return auxilia::AlreadyExistsError("Content already loaded");
  std::ostringstream oss;
  oss << ss.rdbuf();
  const_cast<string_type &>(contents) = oss.str();
  tokens.clear();
  lexeme_views.clear();
  return {};
}

lexer::status_t lexer::lex() {
  while (not is_at_end()) {
    head = cursor;
    next_token();
  }
  add_token(kEndOfFile);
  return {};
}
void lexer::add_identifier_and_keyword() {
  auto value = lex_identifier();
  auto it = keywords.find(value);
  if (it == keywords.end()) {
    dbg(trace, "identifier: {}", value)
    add_token(kIdentifier, value);
    return;
  }
  switch (it->second.type) {
  case kTrue:
    add_token(kTrue, true);
    break;
  case kFalse:
    add_token(kFalse, false);
    break;
  default:
    dbg(trace, "keyword: {}", value)
    add_token(it->second);
  }
}
void lexer::add_number() {
  if (auto value = lex_number(false); !value.empty()) {
    add_token(kNumber, value);
    return;
  }
  dbg(error, "invalid number.")
}
void lexer::add_string() {
  // hard to do...
  auto status = lex_string();
  auto value = string_view_type(contents.data() + head + 1, cursor - head - 2);
  if (status != auxilia::Status::kOk) {
    dbg(error, "Unterminated string.")
    add_lex_error(error_t::kUnterminatedString);
    return;
  }
  dbg(trace, "string value: {}", value)
  add_token(kString, value);
}
void lexer::add_comment() {
  while (peek() != '\n' && !is_at_end())
    get();
}
void lexer::next_token() {
  // token1 token2
  // 			 ^ cursor position
  contract_assert(cursor < contents.size(), "cursor out of range")

  switch (auto c = get()) {
  case '(':
    return add_token(kLeftParen);
  case ')':
    return add_token(kRightParen);
  case '{':
    return add_token(kLeftBrace);
  case '}':
    return add_token(kRightBrace);
  case ',':
    return add_token(kComma);
  case '.':
    return add_token(kDot);
  case '-':
    return add_token(kMinus);
  case '+':
    return add_token(kPlus);
  case ';':
    return add_token(kSemicolon);
  case '*':
    return add_token(kStar);
  case '!':
    return add_token(advance_if_is('=') ? kBangEqual : kBang);
  case '=':
    return add_token(advance_if_is('=') ? kEqualEqual : kEqual);
  case '<':
    return add_token(advance_if_is('=') ? kLessEqual : kLess);
  case '>':
    return add_token(advance_if_is('=') ? kGreaterEqual : kGreater);
  case '/':
    return advance_if_is('/') ? add_comment() : add_token(kSlash);
  default:
    if (whitespace_chars.find(c) != string_view_type::npos)
      return;
    if (newline_chars.find(c) != string_view_type::npos) {
      current_line++;
      return;
    }
    if (c == '"') {
      return add_string();
    }
    // first, numbers(order matters!)
    if (std::isdigit(c)) {
      return add_number();
    }
    // finally, letters
    if (std::isalpha(c) or c == '_') {
      return add_identifier_and_keyword();
    }
    add_lex_error(error_t::kUnexpectedCharacter);
    dbg(error, "unexpected character: {}", c)
  }
}
lexer::char_t lexer::peek(const size_t offset) const {
  if (is_at_end(offset))
    return 0; // equivalent to '\0'
  return contents[cursor + offset];
}
const lexer::char_t &lexer::get(const size_t offset) {
  contract_assert(cursor < contents.size(), "cursor out of range")
  auto &c = contents[cursor];
  cursor += offset;
  return c;
}
bool lexer::advance_if_is(const char_t expected) {
  if (is_at_end() || contents[cursor] != expected)
    return false;
  cursor++;
  return true;
}
bool lexer::is_at_end(const size_t offset) const {
  return cursor + offset >= contents.size();
}
void lexer::add_token(const token_type_t &type, literal_type literal) {
  if (type == kEndOfFile) { // FIXME: lexeme bug at EOF(not critical)
    tokens.emplace_back(type, ""sv, literal_type{}, current_line);
    return;
  }
  auto lexeme = string_view_type(contents.data() + head, cursor - head);
  dbg(trace, "lexeme: {}", lexeme)
  tokens.emplace_back(type, lexeme, std::move(literal), current_line);
  lexeme_views.emplace_back(lexeme);
}
void lexer::add_lex_error(const error_code_t type) {
  dbg(error, "Lexical error: {}", contents.substr(head, cursor - head))
  error_count++;
  return add_token(kLexError, literal_type{error_t{type}});
}
lexer::status_t::Code lexer::lex_string() {
  while (peek() != '"' && !is_at_end()) {
    if (peek() == '\n')
      current_line++; // multiline string, of course we dont want act like C/C++
                      // which will result in a compile error if the string is
                      // not closed at the same current_line.
    get();
  }
  if (is_at_end() && peek() != '"') {
    dbg(error, "Unterminated string.")
    return status_t::kInvalidArgument;
  }
  // "i am a string..."
  // 						      ^ cursor position

  get(); // consume the closing quote.
  return status_t::kOk;
}
auto lexer::lex_number(const bool is_negative) -> literal_type {
  while (std::isdigit(peek())) {
    get();
  }
  bool is_floating_point = false;
  // maybe a '.'?
  if (peek() == '.' && std::isdigit(peek(1))) {
    get(); // consume the '.'
    while (std::isdigit(peek())) {
      get();
    }
    // 123.456_
    // 		    ^ cursor position
    is_floating_point = true;
  }
  // 789_
  //    ^ cursor position
  auto value = contents.substr(head, cursor - head);
  /// @note codecrafter's test view all of it as double
  (void)is_floating_point;
  return to_number<long double>(value);
  // if (is_negative && !is_floating_point) {
  //   return to_number<long long int>(value);
  // }
  // if (!is_negative && !is_floating_point) {
  //   return to_number<unsigned long long int>(value);
  // }
  // if (is_negative && is_floating_point) {
  //   return to_number<long double>(value);
  // }
  // return to_number<double>(value);
}
auto lexer::get_tokens() -> tokens_t & { return tokens; }
bool lexer::ok() const noexcept { return !error_count; }
uint_least32_t lexer::error() const noexcept { return error_count; }
lexer::string_view_type lexer::lex_identifier() {
  while (std::isalnum(peek()) ||
         tolerable_chars.find(peek()) != string_view_type::npos) {
    get();
  }
  // 123_abc
  //       ^ cursor position
  auto value = string_view_type(contents.data() + head, cursor - head);
  return value;
}
AC_LOX_API void delete_lexer_fwd(lexer *ptr) { delete ptr; }
} // namespace accat::lox
