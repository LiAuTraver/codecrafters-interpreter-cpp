#pragma once
#include <limits>
#include <source_location>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include "config.hpp"
#include "loxo_fwd.hpp"

namespace net::ancillarycat::utils {
/// @brief A class that represents the status of a function call. it's designed
/// to be as identical as possible to the `absl::Status` class, for
/// `absl::Status` seems to fail to compile with clang++ on Windows.
class LOXO_API NODISCARD_LOXO(Status) Status {
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
  Status(Code,
         string_view = "<no message provided>",
         const std::source_location & = std::source_location::current());
  NODISCARD_LOXO(Status) Status(Status &&) noexcept;
  NODISCARD_LOXO(Status) Status(const Status &);
  Status &operator=(const Status &) = default;
  Status &operator=(Status &&that) noexcept;
  virtual ~Status() = default;

  NODISCARD_LOXO(Status) bool ok() const noexcept;
  Code code() const;
  NODISCARD_LOXO(string_view) string_view message() const;
  NODISCARD_LOXO(source_location) std::source_location location() const;
  NODISCARD_LOXO(stacktrace) string stacktrace() const;
  /// @note   never do unnecessary `noexcept`;
  /// it'll jeopardize the debugging process and call `std::abort()`.
  void ignore_error() const;
  NODISCARD_LOXO(string) inline string from_source_location() const;

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
    // LOXO_API // <- no need, template class
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
  constexpr explicit operator bool(this auto&& self)noexcept { return self.ok(); }
  value_type operator*(this auto &&self)noexcept { return self.my_value; }
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
LOXO_API Status
OkStatus(const std::source_location & = std::source_location::current());

NODISCARD_LOXO(Status)
LOXO_API Status AlreadyExistsError(
    string_view,
    const std::source_location & = std::source_location::current());
NODISCARD_LOXO(Status)
LOXO_API Status FileNotFoundError(
    string_view,
    const std::source_location & = std::source_location::current());
NODISCARD_LOXO(Status)
LOXO_API Status
UnknownError(string_view,
             const std::source_location & = std::source_location::current());
NODISCARD_LOXO(Status)
LOXO_API Status PermissionDeniedError(
    string_view,
    const std::source_location & = std::source_location::current());
NODISCARD_LOXO(Status)
LOXO_API Status
InvalidArgument(string_view,
                const std::source_location & = std::source_location::current());
NODISCARD_LOXO(Status)
LOXO_API Status
CommandNotFound(string_view,
                const std::source_location & = std::source_location::current());
NODISCARD_LOXO(Status)
LOXO_API Status
EmptyInput(string_view,
           const std::source_location & = std::source_location::current());
NODISCARD_LOXO(Status)
LOXO_API Status
ParseError(string_view,
           const std::source_location & = std::source_location::current());
NODISCARD_LOXO(Status)
LOXO_API Status NotImplementedError(
    string_view,
    const std::source_location & = std::source_location::current());
} // namespace net::ancillarycat::utils
