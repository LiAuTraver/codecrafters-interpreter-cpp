#pragma once
#include <string>
#include <string_view>
#include <cstdint>

#include "config.hpp"
#include "loxo_fwd.hpp"

namespace net::ancillarycat::loxograph {
class lex_error {
public:
  enum type_t : uint8_t {
    kMonostate = 0,
    kUnexpectedCharacter,
    kUnterminatedString,
  };

public:
  using string_t = StringType;
  using string_view_t = StringViewType;

public:
  inline lex_error() = default;
  inline lex_error(const lex_error &) = default;
  inline lex_error(lex_error &&that) = default;
  inline lex_error &operator=(const lex_error &) = default;
  inline lex_error &operator=(lex_error &&that) = default;
  inline explicit lex_error(type_t type) : type(type) {}

public:
  type_t type = kMonostate;

public:
  string_t to_string(const string_view_t lexeme_sv = "<uninitialized>"sv,
                     const uint_least32_t line = 0) const {
    // contract_assert(current_line && (bool)"Line number must not be zero");
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
};
} // namespace net::ancillarycat::loxograph
