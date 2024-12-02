#pragma once

#include "config.hpp"
#include "loxo_fwd.hpp"

namespace net::ancillarycat::loxograph {
class parse_error {
public:
  enum error_type_t : uint8_t {
    kMonostate = 0,
    kMissingSemicolon = 1,
    kMissingComma = 2,
    kMissingParenthesis = 3,
    kMissingBrace = 4,
    kUnknownError = std::numeric_limits<uint8_t>::max(),
  };
  using string_type = std::string;

public:
  parse_error() = default;
  parse_error(const error_type_t &error, const string_type &message)
      : my_error(error), my_message(message) {}
  parse_error(const error_type_t &error, string_type &&message)
      : my_error(error), my_message(std::move(message)) {}
  parse_error(const parse_error &other) = default;
  parse_error(parse_error &&other) noexcept = default;
  auto operator=(const parse_error &other) -> parse_error & = default;
  auto operator=(parse_error &&other) noexcept -> parse_error & = default;
  ~parse_error() = default;

public:
  utils::string_view message() const noexcept { return my_message.data(); }
  error_type_t error() const noexcept { return my_error; }

private:
  error_type_t my_error = kMonostate;
  string_type my_message = "<no message provided>";
};
} // namespace net::ancillarycat::loxograph
