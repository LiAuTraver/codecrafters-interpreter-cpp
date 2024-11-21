#pragma once
#include <source_location>

#include "config.hpp"
#include "loxo_fwd.hpp"

namespace net::ancillarycat::utils {
class LOXOGRAPH_API nodiscard_msg(Status) Status {
public:
  enum nodiscard_msg(Code) Code : uint8_t {
    kOkStatus = 0,
    kError = 1, // represents a generic error
    kAlreadyExistsError = 2,
    kNotFoundError = 3,
    kMovedFrom = 4,
    kPermissionDeniedError = 5,
    kUnknownError = std::numeric_limits<uint8_t>::max()
  };
  nodiscard_msg(Status) Status()
      : my_code(kOkStatus), my_message("<default initialized>"),
        my_location(std::source_location::current()) {}
  nodiscard_msg(Status) explicit Status(
      Code, string_view = "<no message provided>",
      std::source_location = std::source_location::current());
  nodiscard_msg(Status) Status(Status &&) noexcept;
  nodiscard_msg(Status) Status(const Status &);
  Status &operator=(const Status &that);
  Status &operator=(Status &&that) noexcept;

  nodiscard_msg(Status) bool ok() const;
  Code code() const;
  nodiscard_msg(string_view) string_view message() const;
  nodiscard_msg(source_location) std::source_location location() const;
  nodiscard_msg(stacktrace) string stacktrace() const;
  /// @note   never do unnecessary `noexcept`;
  /// it'll jeopardize the debugging process and call `std::abort()`.
  inline constexpr bool ignore_error() const /*noexcept*/;
  nodiscard_msg(string) inline string from_source_location() const;

private:
  Code my_code;
  string my_message;
  std::source_location my_location;
};

nodiscard_msg(Status) Status
    OkStatus(const std::source_location & = std::source_location::current());

nodiscard_msg(Status) Status
    AlreadyExistsError(string_view, const std::source_location & =
                                        std::source_location::current());
nodiscard_msg(Status) Status
    FileNotFoundError(string_view, const std::source_location & =
                                       std::source_location::current());
nodiscard_msg(Status) Status
    UnknownError(string_view, const std::source_location & =
                                  std::source_location::current());
nodiscard_msg(Status) Status
    PermissionDeniedError(string_view, const std::source_location & =
                                           std::source_location::current());
} // namespace net::ancillarycat::utils