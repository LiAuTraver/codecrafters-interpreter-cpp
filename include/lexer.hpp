#pragma once
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
#include <istream>
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
#include "Token.hpp"
#include "config.hpp"
#include "file_reader.hpp"
#include "loxo_fwd.hpp"
#include "status.hpp"

/// @namespace net::ancillarycat::loxograph
namespace net::ancillarycat::loxograph {
// template <typename StringType = std::string,
//           typename StringViewType = std::string_view,
//           typename PathType = std::filesystem::path,
//           typename BooleanType = bool, typename StatusType = Status>
class lexer {
public:
  using size_type = typename StringType::size_type;
  using string_type = StringType;
  using string_view_type = StringViewType;
  using path_type = PathType;
  using boolean_type = BooleanType;
  using status_t = StatusType;
  using token_t = Token;
  using token_type_t = token_t::token_type;
  using tokens_t = std::vector<token_t>;
  using lexeme_views_t = std::vector<string_view_type>;
  using file_reader_t = file_reader</*dafault template arguments*/>;
  using char_t = typename string_type::value_type;

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
  status_t load(const path_type &);
  /// @brief load the contents of the file
  /// @param content the contents of the file
  /// @return OkStatus() if successful, AlreadyExistsError() otherwise
  status_t load(string_type &&);
	/// @brief load the contents of the file
  /// @param content the contents of the file
  /// @return OkStatus() if successful, AlreadyExistsError() otherwise
  status_t load(const std::istream&);
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
  bool is_at_end(size_t = 0) const;
  void add_token(token_type_t, std::any = std::any());
  auto lex_string() -> string_view_type;
  auto lex_identifier() -> string_view_type;
  auto lex_number(boolean_type) -> std::any;

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
             std::convertible_to<Predicate, boolean_type>;

public:
  nodiscard_msg(token_views_t) const tokens_t &get_tokens() const {
    return tokens;
  }

private:
  /// @brief convert a string to a number
  /// @tparam Num the number type
  /// @param value the string to convert
  /// @return the number if successful, std::any() otherwise
  template <typename Num>
    requires std::is_arithmetic_v<Num>
  std::any to_number(string_view_type value);

private:
  /// @brief head of a token
  size_type head = 0;
  /// @brief current cursor position
  size_type cursor = 0;
  /// @brief the contents of the file
  const string_type contents;
  /// @brief tokens
  tokens_t tokens;
  /// @brief lexme views(non-owning)
  lexeme_views_t lexeme_views;
  /// @brief current source line number
  uint_least32_t line = 1;
};
} // namespace net::ancillarycat::loxograph