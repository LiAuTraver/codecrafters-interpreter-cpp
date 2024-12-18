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
class [[nodiscard]] Status {
public:
  enum [[nodiscard]] Code : uint8_t {
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
    kNotImplementedError = 11,

    /// @return values specific for interpreter, not an `error` but a `status`
    /// @note `.ok()` will return `false`.
    kReturning = 12,

    kUnknownError = std::numeric_limits<uint8_t>::max(),
  };

public:
  [[nodiscard]]
  Status()
      : my_code(kOkStatus), my_message("<default initialized>"),
        my_location(std::source_location::current()) {}

  [[nodiscard]]
  Status(Code code,
         string_view message = "<no message provided>",
         const std::source_location &location = std::source_location::current())
      : my_code(code), my_message(message), my_location(location) {}

  [[nodiscard]]
  Status(Status &&that) noexcept
      : my_code(that.my_code), my_message(std::move(that.my_message)),
        my_location(that.my_location) {
    that.my_code = kMovedFrom;
    that.my_message = "This status has been moved from."s;
    that.my_location = std::source_location::current();
  }

  [[nodiscard]]
  Status(const Status &that) = default;
  auto operator=(const Status &that) -> Status & = default;

  Status &operator=(Status &&that) noexcept {
    my_code = that.my_code;
    my_message = std::move(that.my_message);
    my_location = that.my_location;
    that.my_code = kMovedFrom;
    that.my_message = "This status has been moved from."s;
    that.my_location = std::source_location::current();
    return *this;
  }
  inline AC_UTILS_CONSTEXPR_IF_NOT_MSVC explicit
  operator bool() const noexcept {
    return this->ok();
  }

  [[nodiscard]] bool ok() const noexcept { return my_code == kOkStatus; }
  Code code() const { return my_code; }
  [[nodiscard]] string_view message() const { return my_message; }
  [[nodiscard]] std::source_location location() const { return my_location; }
  [[nodiscard]] string stacktrace() const { return AC_UTILS_STACKTRACE; }
  void ignore_error() const { contract_assert(ok()); }
  [[nodiscard]] inline string from_source_location() const {
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
template <Storable Ty> class [[nodiscard]] StatusOr : public Status {
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
    contract_assert(self.ok() or self.code() == Status::kReturning);
    return self.my_value;
  }

  value_type value_or(this auto &&self, const value_type &default_value) {
    return self.ok() ? self.my_value : default_value;
  }
  inline constexpr value_type operator*(this auto &&self) noexcept {
    return self.my_value;
  }
  inline constexpr auto
  operator->(this auto &&self) noexcept -> decltype(auto) {
    return std::addressof(self.my_value);
  }
  base_type as_status(this auto &&self) noexcept {
    return Status{self.my_code, self.my_message, self.my_location};
  }

  auto reset(Ty &&value) noexcept {
    my_value = std::move(value);
    my_code = kOkStatus;
    my_message.clear();
    my_location = std::source_location::current();
    return *this;
  }

private:
  value_type my_value;
};

[[nodiscard]]
inline Status OkStatus(
    const std::source_location &location = std::source_location::current()) {
  return {Status::kOkStatus, "OkStatus", location};
}

[[nodiscard]]
inline Status AlreadyExistsError(
    const std::string_view message,
    const std::source_location &location = std::source_location::current()) {
  return {Status::kAlreadyExistsError, message, location};
}

[[nodiscard]]
inline Status FileNotFoundError(
    const std::string_view message,
    const std::source_location &location = std::source_location::current()) {
  return {Status::kNotFoundError, message, location};
}

[[nodiscard]]
inline Status NotFoundError(
    const std::string_view message,
    const std::source_location &location = std::source_location::current()) {
  return {Status::kNotFoundError, message, location};
}

[[nodiscard]]
inline Status UnknownError(
    const std::string_view message,
    const std::source_location &location = std::source_location::current()) {
  return {Status::kUnknownError, message, location};
}

[[nodiscard]]
inline Status PermissionDeniedError(
    const std::string_view message,
    const std::source_location &location = std::source_location::current()) {
  return {Status::kPermissionDeniedError, message, location};
}

[[nodiscard]]
inline Status InvalidArgument(
    const string_view message,
    const std::source_location &location = std::source_location::current()) {
  return {Status::kInvalidArgument, message, location};
}

[[nodiscard]]
inline Status CommandNotFound(
    const string_view message,
    const std::source_location &location = std::source_location::current()) {
  return {Status::kCommandNotFound, message, location};
}

[[nodiscard]]
inline Status EmptyInput(
    const string_view message,
    const std::source_location &location = std::source_location::current()) {
  return {Status::kEmptyInput, message, location};
}

[[nodiscard]]
inline Status ParseError(
    const string_view message,
    const std::source_location &location = std::source_location::current()) {
  return {Status::kParseError, message, location};
}

[[nodiscard]]
inline Status NotImplementedError(
    const string_view message,
    const std::source_location &location = std::source_location::current()) {
  return {Status::kNotImplementedError, message, location};
}
} // namespace net::ancillarycat::utils
