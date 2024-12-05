#pragma once
#include <cstdint>
#include <string>
#include <string_view>

#include "loxo_fwd.hpp"

#include <net/ancillarycat/utils/file_reader.hpp>

namespace net::ancillarycat::loxo {
class LOXO_API lex_error {
public:
  enum type_t : uint8_t {
    kMonostate = 0,
    kUnexpectedCharacter,
    kUnterminatedString,
  };

public:
  using string_t = utils::string;
  using string_view_t = utils::string_view;

public:
  constexpr lex_error() = default;
  constexpr lex_error(const lex_error &) = default;
  constexpr lex_error(lex_error &&that) = default;
  constexpr lex_error &operator=(const lex_error &) = default;
  constexpr lex_error &operator=(lex_error &&that) = default;
  explicit lex_error(const type_t type) : type(type) {}

public:
  type_t type = kMonostate;

public:
  string_t to_string(string_view_t lexeme_sv = ""sv,
                     uint_least32_t line = 0) const;
};
inline lex_error::string_t
lex_error::to_string(const string_view_t lexeme_sv,
                     const uint_least32_t line) const {
  if (!line)
    dbg(warn, "line number is 0");
  if (lexeme_sv.empty())
    dbg(warn, "lexeme is empty");

  auto msg = string_t{};
  switch (type) {
  case kMonostate:
    dbg(critical, "Shouldn't happen: type is kMonostate");
    msg = "Internal error";
    break;
  case kUnexpectedCharacter:
    msg = utils::format("Unexpected character: {}", lexeme_sv.data());
    break;
  case kUnterminatedString:
    msg = "Unterminated string.";
    break;
  }
  auto str = utils::format("[line {}] Error: {}", line, msg);
  dbg(error, "{}", str);
  return str;
}

inline auto
format_to(const lex_error &lex_error,
          const utils::FormatPolicy format_policy = utils::kDefault) {
  return lex_error.to_string();
}
} // namespace net::ancillarycat::loxo
