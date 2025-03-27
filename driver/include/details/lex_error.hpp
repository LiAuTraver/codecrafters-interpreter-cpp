#pragma once
#include <cstdint>
#include <string>
#include <string_view>

#include "accat/auxilia/details/format.hpp"
#include "details/loxo_fwd.hpp"

namespace accat::loxo {
class LOXO_API lex_error {
public:
  enum type_t : uint8_t {
    kMonostate = 0,
    kUnexpectedCharacter,
    kUnterminatedString,
  };

public:
  using string_t = auxilia::string;
  using string_view_t = auxilia::string_view;

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
    dbg(warn, "line number is 0")
  if (lexeme_sv.empty()){
    dbg(warn, "lexeme is empty")}

  auto my_msg = string_t{};

  switch (type) {
  case kMonostate:
    dbg(critical, "Shouldn't happen: type is kMonostate")
    my_msg = "Internal error";
    break;
  case kUnexpectedCharacter:
    my_msg = auxilia::format("Unexpected character: {}", lexeme_sv.data());
    break;
  case kUnterminatedString:
    my_msg = "Unterminated string.";
    break;
  }
  return auxilia::format("[line {}] Error: {}", line, my_msg);
}

inline auto
format_to(const lex_error &lex_error,
          const auxilia::FormatPolicy format_policy = auxilia::FormatPolicy::kDefault) {
  return lex_error.to_string();
}
} // namespace accat::loxo
