#pragma once

#include <cstddef>
#include <cstdint>
#include <source_location>
#include <string>
#include <string_view>
#include <vector>

#include <accat/auxilia/auxilia.hpp>

#include "details/loxo_fwd.hpp"
#include "details/lex_error.hpp"
#include "Token.hpp"

/// @namespace accat::loxo
namespace accat::loxo {
class LOXO_API lexer {
public:
  using size_type = typename auxilia::string::size_type;
  using string_type = auxilia::string;
  using string_view_type = auxilia::string_view;
  using path_type = auxilia::path;
  using status_t = auxilia::Status;
  using token_t = Token;
  using token_type_t = token_t::token_type;
  using tokens_t = std::vector<token_t>;
  using lexeme_views_t = std::vector<string_view_type>;
  using char_t = typename string_type::value_type;
  using error_t = lex_error;
  using error_code_t = typename error_t::type_t;
  using enum token_type_t::type_t;
  using literal_type = token_t::literal_type;

public:
  lexer() = default;
  lexer(const lexer &other) = delete;
  lexer(lexer &&other) noexcept;
  lexer &operator=(const lexer &other) = delete;
  lexer &operator=(lexer &&other) noexcept;
  ~lexer() = default;

public:
  /// @brief load the contents of the file
  /// @return OkStatus() if successful, NotFoundError() otherwise
  status_t load(const path_type &) const;
  /// @copydoc load(const path_type &)
  status_t load(const std::istream &);
  /// @brief lex the contents of the file
  /// @return OkStatus() if successful, NotFoundError() otherwise
  status_t lex();
  auto get_tokens() -> tokens_t &;
  bool ok() const noexcept;
  uint_least32_t error() const noexcept;

private:
  void add_identifier_and_keyword();
  void add_number();
  void add_string();
  void add_comment();
  void next_token();
  void add_token(const token_type_t &, literal_type = {});
  void add_lex_error(lex_error::type_t = error_t::kMonostate);
  bool is_at_end(size_t = 0) const;
  auto lex_string() -> lexer::status_t::Code;
  auto lex_identifier() -> string_view_type;
  auto lex_number(bool) -> literal_type;

private:
  /// @brief lookaheads; we have only consumed the character before the cursor
  char_t peek(size_t offset = 0) const;
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
             std::convertible_to<Predicate, bool>;

public:
  [[nodiscard]] const tokens_t &get_tokens() const { return tokens; }

private:
  /// @brief head of a token
  size_type head = 0;
  /// @brief current cursor position
  size_type cursor = 0;
  /// @brief the contents of the file
  const string_type contents = string_type();
  /// @brief lexme views(non-owning)
  lexeme_views_t lexeme_views = lexeme_views_t();
  /// @brief current source line number
  uint_least32_t current_line = 1;
  /// @brief tokens
  tokens_t tokens = tokens_t();
  /// @brief errors
  uint_least32_t error_count = 0;

private:
  friend LOXO_API void delete_lexer_fwd(lexer *);
};
} // namespace accat::loxo
