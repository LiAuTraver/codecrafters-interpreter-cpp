#pragma once
#include <limits>
#include <source_location>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include "config.hpp"
#include "format.hpp"

namespace net::ancillarycat::utils {
/// @brief A class that represents the status of a function call. it's designed
/// to be as identical as possible to the `absl::Status` class, for
/// `absl::Status` seems to fail to compile with clang++ on Windows.
class NODISCARD_LOXO(Status) Status {
public:
  enum NODISCARD_LOXO(Code) Code : uint8_t {
    kOkStatus = 0,
    kError = 1, // represents a generic error
    kAlreadyExistsError = 2,
    kNotFoundError = 3,
    kMovedFrom = 4,
    kPermissionDeniedError = 5,
    kInvalidArgument = 6,
    kCommandNotFound = 7,
    kEmptyInput = 8,
    kParseError = 9,
    kNotImplementedError = 10,
    kUnknownError = std::numeric_limits<uint8_t>::max(),
  };

public:
  NODISCARD_LOXO(Status)
  Status()
      : my_code(kOkStatus), my_message("<default initialized>"),
        my_location(std::source_location::current()) {}

  NODISCARD_LOXO(Status)
  Status(Code code,
         string_view message = "<no message provided>",
         const std::source_location &location = std::source_location::current())
      : my_code(code), my_message(message), my_location(location) {}

  NODISCARD_LOXO(Status) Status(Status &&that) noexcept
      : my_code(that.my_code), my_message(std::move(that.my_message)),
        my_location(that.my_location) {
    that.my_code = kMovedFrom;
    that.my_message = "This status has been moved from."sv;
    that.my_location = std::source_location::current();
  }

  NODISCARD_LOXO(Status) Status(const Status &that)
      : my_code(that.my_code), my_message(that.my_message),
        my_location(that.my_location) {}

  Status &operator=(Status &&that) noexcept {
    my_code = that.my_code;
    my_message = std::move(that.my_message);
    my_location = that.my_location;
    that.my_code = kMovedFrom;
    that.my_message = "This status has been moved from.";
    that.my_location = std::source_location::current();
    return *this;
  }

  NODISCARD_LOXO(Status) bool ok() const noexcept { return my_code == kOkStatus; }
  Code code() const { return my_code; }
  NODISCARD_LOXO(string_view) string_view message() const { return my_message; }
  NODISCARD_LOXO(source_location) std::source_location location() const { return my_location; }
  NODISCARD_LOXO(stacktrace) string stacktrace() const { return LOXO_STACKTRACE; }
  void ignore_error() const { contract_assert(ok()); }
  NODISCARD_LOXO(string) inline string from_source_location() const {
    return utils::format("file {0}\n"
                         "              function {1},\n"
                         "              Ln {2} Col {3}\n",
                         my_location.file_name(),
                         my_location.function_name(),
                         my_location.line(),
                         my_location.column());
  }

public:
  Code my_code;
  string my_message;
  std::source_location my_location;
};
/// @brief A class that represents the status of a function call, or a
/// value.
///         it's designed to be as identical as possible to the
///         `absl::StatusOr` class.
/// @tparam Ty the type of the value
template <Storable Ty>
class
    //  // <- no need, template class
    NODISCARD_LOXO(StatusOr) StatusOr : public Status {
public:
  using base_type = Status;
  using value_type = Ty;

public:
  constexpr StatusOr() = default;
  StatusOr(const Status &status) : base_type(status) {}
  StatusOr(Status &&status) : base_type(std::move(status)) {}
  StatusOr(const value_type &value) : base_type(kOkStatus), my_value(value) {}
  StatusOr(value_type &&value)
      : base_type(kOkStatus), my_value(std::move(value)) {}
  StatusOr(const Status &status, const value_type &value)
      : base_type(status), my_value(value) {}
  StatusOr(Status &&status, value_type &&value)
      : base_type(std::move(status)), my_value(std::move(value)) {}

  StatusOr(const StatusOr &that) : base_type(that), my_value(that.my_value) {}
  StatusOr(StatusOr &&that) noexcept
      : base_type(std::move(that)), my_value(std::move(that.my_value)) {}

  StatusOr &operator=(const StatusOr &that) {
    base_type::operator=(that);
    my_value = that.my_value;
    return *this;
  }
  StatusOr &operator=(StatusOr &&that) noexcept {
    base_type::operator=(std::move(that));
    my_value = std::move(that.my_value);
    return *this;
  }

  value_type value(this auto &&self) {
    contract_assert(self.ok());
    return self.my_value;
  }

  value_type value_or(this auto &&self, const value_type &default_value) {
    return self.ok() ? self.my_value : default_value;
  }
  constexpr explicit operator bool(this auto &&self) noexcept {
    return self.ok();
  }
  value_type operator*(this auto &&self) noexcept { return self.my_value; }
  auto operator->(this auto &&self) noexcept -> decltype(auto) {
    return std::addressof(self.my_value);
  }
  base_type as_status(this auto &&self) noexcept {
    return Status{self.my_code, self.my_message, self.my_location};
  }

private:
  value_type my_value;
};

NODISCARD_LOXO(Status)
inline Status OkStatus(const std::source_location &location = std::source_location::current()) {
  return {Status::kOkStatus, "OkStatus", location};
}

NODISCARD_LOXO(Status)
inline Status AlreadyExistsError(const std::string_view message,
                                 const std::source_location &location = std::source_location::current()) {
  return {Status::kAlreadyExistsError, message, location};
}

NODISCARD_LOXO(Status)
inline Status FileNotFoundError(const std::string_view message,
                                const std::source_location &location = std::source_location::current()) {
  return {Status::kNotFoundError, message, location};
}

NODISCARD_LOXO(Status)
inline Status UnknownError(const std::string_view message,
                           const std::source_location &location = std::source_location::current()) {
  return {Status::kUnknownError, message, location};
}

NODISCARD_LOXO(Status)
inline Status PermissionDeniedError(const std::string_view message,
                                    const std::source_location &location = std::source_location::current()) {
  return {Status::kPermissionDeniedError, message, location};
}

NODISCARD_LOXO(Status)
inline Status InvalidArgument(const string_view message,
                              const std::source_location &location = std::source_location::current()) {
  return {Status::kInvalidArgument, message, location};
}

NODISCARD_LOXO(Status)
inline Status CommandNotFound(const string_view message,
                              const std::source_location &location = std::source_location::current()) {
  return {Status::kCommandNotFound, message, location};
}

NODISCARD_LOXO(Status)
inline Status EmptyInput(const string_view message,
                         const std::source_location &location = std::source_location::current()) {
  return {Status::kEmptyInput, message, location};
}

NODISCARD_LOXO(Status)
inline Status ParseError(const string_view message,
                         const std::source_location &location = std::source_location::current()) {
  return {Status::kParseError, message, location};
}

NODISCARD_LOXO(Status)
inline Status NotImplementedError(const string_view message,
                                  const std::source_location &location = std::source_location::current()) {
  return {Status::kNotImplementedError, message, location};
}

} // namespace net::ancillarycat::utils
