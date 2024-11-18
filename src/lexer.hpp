#include <algorithm>
#include <any>
#include <array>
#include <cctype>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <locale>
#include <optional>
#include <sstream>
#include <stacktrace>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>
#include "config.hpp"
#include "file_reader.hpp"
#include "loxo_fwd.hpp"
#include "status.hpp"
#include "token.hpp"

/// @namespace net::ancillarycat::loxograph
namespace net::ancillarycat::loxograph {
using namespace net::ancillarycat::utils;
// template <typename StringType = std::string,
//           typename StringViewType = std::string_view,
//           typename PathType = std::filesystem::path,
//           typename BooleanType = bool, typename StatusType = Status>
class lexer {
public:
  using size_type = typename StringType::size_type;
  using string_t = StringType;
  using string_view_t = StringViewType;
  using path_t = PathType;
  using boolean_t = BooleanType;
  using status_t = StatusType;
  using token_t = Token;
  using token_type_t = token_t::token_type;
  using tokens_t = std::vector<token_t>;
  using lexeme_views_t = std::vector<string_view_t>;
  using file_reader_t = file_reader;
  using char_t = typename string_t::value_type;

public:
  inline explicit constexpr lexer() = default;

  inline constexpr lexer(const lexer &other) = delete;
  inline constexpr lexer(lexer &&other) = delete;

  inline constexpr lexer &operator=(const lexer &other) = delete;
  inline constexpr lexer &operator=(lexer &&other) = delete;

  inline constexpr ~lexer() = default;

public:
  /// @brief load the contents of the file
  /// @param filepath the path to the file
  /// @return OkStatus() if successful, NotFoundError() otherwise
  status_t load(const path_t &filepath);
  /// @brief load the contents of the file
  /// @param content the contents of the file
  /// @return OkStatus() if successful, AlreadyExistsError() otherwise
  status_t load(string_t &&content);
  /// @brief lex the contents of the file
  /// @return OkStatus() if successful, NotFoundError() otherwise
  status_t lex();
  auto get_tokens() -> tokens_t;

private:
  void add_identifier();
  void add_number();
  void add_string();
  void add_comment();
  void next_token();
  bool is_at_end(size_t offset = 0) const;
  void add_token(token_type_t type, std::any literal = std::any());
  auto lex_string() -> string_view_t;
  auto lex_identifier() -> string_view_t;
  auto lex_number(bool is_negative) -> std::any;

private:
  /// @brief lookaheads; we have only consumed the character before the cursor
  char_t peek(size_t offset = 0);
  /// @brief get current character and advance the cursor
  /// @note does not check if @code cursor + offset >= contents.size() @endcode
  const char_t &get(size_t offset = 1);

  /// @brief advance the cursor if the character is the expected character
  /// @param expected the expected character
  /// @return true if the character is the expected character and the cursor is
  /// advanced, false otherwise
  bool advance_if_is(char_t expected);

  /// @brief advance the cursor if the predicate is true
  /// @tparam Predicate the predicate to check
  /// @param predicate the predicate to check
  /// @return true if the predicate is true and the cursor is advanced, false
  template <typename Predicate>
  bool advance_if(Predicate &&predicate)
    requires std::invocable<Predicate, char_t> &&
             std::convertible_to<Predicate, boolean_t>;

public:
  nodiscard_msg(token_views_t) const tokens_t& get_tokens() const {
    return tokens;
  }

private:
  /// @brief convert a string to a number
  /// @tparam Num the number type
  /// @param value the string to convert
  /// @return the number if successful, std::any() otherwise
  template <typename Num>
    requires std::is_arithmetic_v<Num>
  std::any to_number(string_view_t value);

private:
  /// @brief head of a token
  size_type head = 0;
  /// @brief current cursor position
  size_type cursor = 0;
  /// @brief the contents of the file
  const string_t contents;
  /// @brief tokens
  tokens_t tokens;
  /// @brief lexme views(non-owning)
  lexeme_views_t lexeme_views;
  /// @brief current source line number
  uint_least32_t line = 1;
};
} // namespace net::ancillarycat::loxograph
/// @namespace net::ancillarycat::loxograph
namespace net::ancillarycat::loxograph {
template <typename Predicate>
bool lexer::advance_if(Predicate &&predicate)
  requires std::invocable<Predicate, char_t> &&
           std::convertible_to<Predicate, boolean_t>
{
  if (is_at_end() || !predicate(contents[cursor]))
    return false;
  cursor++;
  return true;
}
template <typename Num>
  requires std::is_arithmetic_v<Num>
std::any lexer::to_number(string_view_t value) {
  Num number;
  if (const auto &[p, ec] =
          std::from_chars(value.data(), value.data() + value.size(), number);
      ec != std::errc()) {
    dbg(error, "Unable to convert string to number: {}", value);
    dbg(error, "Error code: {}", std::to_underlying(ec));
    dbg(error, "Error position: {}", p);
    return std::any();
  }
  return {number};
}
inline lexer::status_t lexer::load(const path_t &filepath) {
  if (not contents.empty())
    return AlreadyExistsError("File already loaded");
  file_reader_t reader(filepath);
  const_cast<string_t &>(contents) = reader.get_contents();
  if (contents.empty())
    return NotFoundError("Unable to open file: " + filepath.string());
  return OkStatus();
}
inline lexer::status_t lexer::load(string_t &&content) {
  if (not contents.empty())
    return AlreadyExistsError("Content already loaded");
  const_cast<string_t &>(contents) = std::move(content);
  tokens.clear();
  lexeme_views.clear();
  return OkStatus();
}
inline lexer::status_t lexer::lex() {
  if (contents.empty())
    return NotFoundError("No content to lex");
  while (not is_at_end()) {
    head = cursor;
    next_token();
  }
  add_token(TokenType::kEndOfFile);
  return OkStatus();
}
inline void lexer::add_identifier() {
  auto value = lex_identifier();
  if (auto it = keywords.find(value); it != keywords.end()) {
    add_token(it->second);
    return;
  }
  add_token(TokenType::kIdentifier, value);
  return;
}
inline void lexer::add_number() {
  auto value = lex_number(false);
  if (!value.has_value()) {
    dbg(error, "invalid number.");
    return;
  }
  add_token(TokenType::kNumber, value);
  return;
}
inline void lexer::add_string() {
  auto value = lex_string();
  add_token(TokenType::kString, value);
}
inline void lexer::add_comment() {
  while (peek() != '\n' && !is_at_end())
    get();
}
inline void lexer::next_token() {
  // token1 token2
  // 			 ^ cursor position
  contract_assert(cursor < contents.size());
  auto c = get();
  dbg(info, "c: {}", c);

  switch (c) {
  case '(':
    return add_token(TokenType::kLeftParen);
  case ')':
    return add_token(TokenType::kRightParen);
  case '{':
    return add_token(TokenType::kLeftBrace);
  case '}':
    return add_token(TokenType::kRightBrace);
  case ',':
    return add_token(TokenType::kComma);
  case '.':
    return add_token(TokenType::kDot);
  case '-':
    // todo: negative number
    return add_token(TokenType::kMinus);
  case '+':
    return add_token(TokenType::kPlus);
  case ';':
    return add_token(TokenType::kSemicolon);
  case '*':
    return add_token(TokenType::kStar);
  case '!':
    return add_token(advance_if_is('=') ? TokenType::kBangEqual
                                        : TokenType::kBang);
  case '=':
    return add_token(advance_if_is('=') ? TokenType::kEqualEqual
                                        : TokenType::kEqual);
  case '<':
    return add_token(advance_if_is('=') ? TokenType::kLessEqual
                                        : TokenType::kLess);
  case '>':
    return add_token(advance_if_is('=') ? TokenType::kGreaterEqual
                                        : TokenType::kGreater);
  case '/':
    return advance_if_is('/') ? add_comment() : add_token(TokenType::kSlash);
  default:
    if (whitespace_chars.find(c) != string_view_t::npos)
      return;
    if (newline_chars.find(c) != string_view_t::npos) {
      line++;
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
    return dbg(error, "unexpected character: {}", c);
  }
}
inline lexer::char_t lexer::peek(size_t offset) {
  if (is_at_end(offset))
    return 0; // equivalent to '\0'
  return contents[cursor + offset];
}
inline const lexer::char_t &lexer::get(size_t offset) {
  contract_assert(cursor < contents.size());
  auto &c = contents[cursor];
  cursor += offset;
  return c;
}
inline bool lexer::advance_if_is(char_t expected) {
  if (is_at_end() || contents[cursor] != expected)
    return false;
  cursor++;
  return true;
}
inline bool lexer::is_at_end(size_t offset) const {
  return cursor + offset >= contents.size();
}
inline void lexer::add_token(token_type_t type, std::any literal) {
  string_view_t lexeme = string_view_t(contents.data() + head, cursor - head);
  dbg(info, "lexeme: {}", lexeme);
  token_t token{type, lexeme, literal, line};
  tokens.push_back(token);
        lexeme_views.push_back(lexeme);
}
inline lexer::string_view_t lexer::lex_string() {
  while (peek() != '"' && !is_at_end()) {
    if (peek() == '\n') {
      line++; // multiline string, of course we dont want act like C/C++ which
              // will result in a compile error if the string is not closed at
              // the same line.
    }
    get();
  }
  if (is_at_end()) {
    dbg(error, "Unterminated string.");
  }
  // "i am a string..."
  // 						     ^ cursor position
  get(); // consume the closing quote.
  auto value = string_view_t(contents.data() + head + 1, cursor - head - 2);
  dbg(info, "string value: {}", value);
  return value;
}
inline std::any lexer::lex_number(bool is_negative) {
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
  if (is_negative && !is_floating_point) {
    return to_number<long long int>(value);
  }
  if (!is_negative && !is_floating_point) {
    return to_number<unsigned long long int>(value);
  }
  if (is_negative && is_floating_point) {
    return to_number<long double>(value);
  }
  return to_number<double>(value);
}
inline auto lexer::get_tokens() -> lexer::tokens_t {
        return tokens;
}
inline lexer::string_view_t lexer::lex_identifier() {
  while (std::isalnum(peek(), std::locale()) ||
         tolerable_chars.find(peek()) != string_view_t::npos) {
    get();
  }
  // 123_abc
  //       ^ cursor position
  auto value = string_view_t(contents.data() + head, cursor - head);
  dbg(info, "identifier: {}", value);
  return value;
}
} // namespace net::ancillarycat::loxograph