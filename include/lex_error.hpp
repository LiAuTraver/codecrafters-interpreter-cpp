#pragma once
#include <string>
#include <string_view>
#include <format>
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
  inline lex_error(type_t type) : type(type) {}

public:
  type_t type = kMonostate;

public:
  string_t to_string(const string_view_t lexeme_sv = "stray lexeme"sv,
                     const uint_least32_t line = 0) const {
    // contract_assert(current_line && (bool)"Line number must not be zero");
    auto msg = string_t{};
    switch (type) {
    case kMonostate:
      dbg(critical, "Shouldn't happen: type is kMonostate");
      msg = "Internal error";
      break;
    case kUnexpectedCharacter:
      msg = "Unexpected character";
      break;
    case kUnterminatedString:
      msg = "Unterminated string";
      break;
    }
    auto str = format("[line {}] Error: {}: {}", line, msg, lexeme_sv.data());
    dbg(error, "{}", str);
    return str;
  }
};
} // namespace net::ancillarycat::loxograph
// template <>
// struct fmt::formatter<net::ancillarycat::loxograph::lex_error> : fmt::formatter<net::ancillarycat::loxograph::lex_error::string_t>{
//   auto format(net::ancillarycat::loxograph::lex_error lex_error, format_context &ctx) const -> decltype(ctx.out())const  {
//     return fmt::format_to(ctx.out(), "{}", lex_error.to_string());
//   }
// };
