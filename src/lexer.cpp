#include <concepts>
#include <locale>
#include <source_location>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <charconv>
#include <any>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <utility>

#include "config.hpp"
#include "lex_error.hpp"
#include "lexer.hpp"
#include "Token.hpp"

/// @namespace net::ancillarycat::loxograph
namespace net::ancillarycat::loxograph {
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
std::any lexer::to_number(string_view_type value) {
  Num number;
  if (const auto &[p, ec] =
          std::from_chars(value.data(), value.data() + value.size(), number);
      ec != std::errc()) {
    dbg(error, "Unable to convert string to number: {}", value);
    dbg(error, "Error code: {}", std::to_underlying(ec));
    dbg(error, "Error position: {}", p);
    return {};
  }
  return {number};
}
lexer::lexer(lexer &&other) noexcept
    : head(std::exchange(other.head, 0)),
      cursor(std::exchange(other.cursor, 0)),
      contents(std::move(const_cast<string_type&>(other.contents))),
      lexeme_views(std::move(other.lexeme_views)),
      current_line(std::exchange(other.current_line, 1)),
      tokens(std::move(other.tokens)),
      error_count(std::exchange(other.error_count, 0)) {}
lexer &lexer::operator=(lexer &&other) noexcept {
  if (this != &other) {
    head = std::exchange(other.head, 0);
    cursor = std::exchange(other.cursor, 0);
    const_cast<string_type &>(contents) =
        std::move(const_cast<string_type &>(other.contents));
    lexeme_views = std::move(other.lexeme_views);
    current_line = std::exchange(other.current_line, 1);
    tokens = std::move(other.tokens);
    error_count = std::exchange(other.error_count, 0);
  }
  return *this;
}
lexer::status_t lexer::load(const path_type &filepath) const {
  if (not contents.empty())
    return utils::AlreadyExistsError("File already loaded");
  if (not std::filesystem::exists(filepath))
    return utils::FileNotFoundError("File does not exist: " +
                                    filepath.string());
  file_reader_t reader(filepath);
  const_cast<string_type &>(contents) = reader.get_contents();
  return utils::OkStatus();
}
lexer::status_t lexer::load(const std::istream &ss) {
  if (not contents.empty())
    return utils::AlreadyExistsError("Content already loaded");
  std::ostringstream oss;
  oss << ss.rdbuf();
  const_cast<string_type &>(contents) = oss.str();
  tokens.clear();
  lexeme_views.clear();
  return utils::OkStatus();
}

lexer::status_t lexer::lex() {
  while (not is_at_end()) {
    head = cursor;
    next_token();
  }
  add_token(kEndOfFile);
  return utils::OkStatus();
}
void lexer::add_identifier() {
  auto value = lex_identifier();
  if (auto it = keywords.find(value); it != keywords.end()) {
    add_token(it->second);
    return;
  }
  add_token(kIdentifier, value);
}
void lexer::add_number() {
  auto value = lex_number(false);
  if (!value.has_value()) {
    dbg(error, "invalid number.");
    return;
  }
  add_token(kNumber, value);
}
void lexer::add_string() {
  // hard to do...
  auto status = lex_string();
  auto value = string_view_type(contents.data() + head + 1, cursor - head - 2);
  if (status != utils::Status::kOkStatus) {
    dbg(error, "Unterminated string.");
    add_lex_error(error_t::kUnterminatedString);
    return;
  }
  dbg(trace, "string value: {}", value);
  add_token(kString, value);
}
void lexer::add_comment() {
  while (peek() != '\n' && !is_at_end())
    get();
}
void lexer::next_token() {
  // token1 token2
  // 			 ^ cursor position
  contract_assert(cursor < contents.size());

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
    // TODO: negative number
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
    if (std::isdigit(c, std::locale())) {
      return add_number();
    }
    // finally, letters
    if (std::isalpha(c, std::locale()) or c == '_') {
      return add_identifier();
    }
    add_lex_error(error_t::kUnexpectedCharacter);
    dbg(error, "unexpected character: {}", c);
  }
}
lexer::char_t lexer::peek(const size_t offset) const {
  if (is_at_end(offset))
    return 0; // equivalent to '\0'
  return contents[cursor + offset];
}
const lexer::char_t &lexer::get(const size_t offset) {
  contract_assert(cursor < contents.size());
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
void lexer::add_token(token_type_t type, std::any literal) {
  auto lexeme = string_view_type(contents.data() + head, cursor - head);
  dbg(trace, "lexeme: {}", lexeme);
  tokens.emplace_back(type, lexeme, std::move(literal), current_line);
  lexeme_views.emplace_back(lexeme);
}
void lexer::add_lex_error(const error_code_t type) {
  dbg(error, "Lexical error: {}", contents.substr(head, cursor - head));
  error_count++;
  return add_token(kLexError, std::make_any<error_t>(type));
}
lexer::status_t::Code lexer::lex_string() {
  while (peek() != '"' && !is_at_end()) {
    if (peek() == '\n') {
      current_line++; // multiline string, of course we dont want act like C/C++
                      // which will result in a compile error if the string is
                      // not closed at the same current_line.
    }
    get();
  }
  // peek() == '"' || is_at_end()
  if (is_at_end() && peek() != '"') {
    dbg(error, "Unterminated string.");
    return status_t::kError;
  }
  // "i am a string..."
  // 						      ^ cursor position
  else
    get(); // consume the closing quote.
  return status_t::kOkStatus;
}
std::any lexer::lex_number(const bool is_negative) {
  while (std::isdigit(peek(), std::locale())) {
    get();
  }
  bool is_floating_point = false;
  // maybe a '.'?
  if (peek() == '.' && std::isdigit(peek(1), std::locale())) {
    get(); // consume the '.'
    while (std::isdigit(peek(), std::locale())) {
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
  while (std::isalnum(peek(), std::locale()) ||
         tolerable_chars.find(peek()) != string_view_type::npos) {
    get();
  }
  // 123_abc
  //       ^ cursor position
  auto value = string_view_type(contents.data() + head, cursor - head);
  dbg(trace, "identifier: {}", value);
  return value;
}
} // namespace net::ancillarycat::loxograph
