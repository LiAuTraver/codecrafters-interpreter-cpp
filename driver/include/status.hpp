#pragma once
#include <source_location>
#include <string>
#include <string_view>
#include <utility>
#include <limits>

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
    kInvalidArgument = 6,
    kCommandNotFound = 7,
    kEmptyInput = 8,
    kParseError = 9,
    kUnknownError = std::numeric_limits<uint8_t>::max()
  };

public:
  nodiscard_msg(Status) Status()
      : my_code(kOkStatus), my_message("<default initialized>"),
        my_location(std::source_location::current()) {}
  nodiscard_msg(Status)
      Status(Code,
             string_view = "<no message provided>",
             const std::source_location & = std::source_location::current());
  nodiscard_msg(Status) Status(Status &&) noexcept;
  nodiscard_msg(Status) Status(const Status &);
  Status &operator=(const Status &) = default;
  Status &operator=(Status &&that) noexcept;
  virtual ~Status() = default;

  nodiscard_msg(Status) bool ok() const;
  Code code() const;
  nodiscard_msg(string_view) string_view message() const;
  nodiscard_msg(source_location) std::source_location location() const;
  nodiscard_msg(stacktrace) string stacktrace() const;
  /// @note   never do unnecessary `noexcept`;
  /// it'll jeopardize the debugging process and call `std::abort()`.
  virtual void ignore_error() const;
  nodiscard_msg(string) inline string from_source_location() const;

public:
  Code my_code;
  string my_message;
  std::source_location my_location;
};

template <Storable Ty>
class
// LOXOGRAPH_API // <- no need, template class
 nodiscard_msg(StatusOr) StatusOr : public Status {
public:
  using base_type = Status;
  using value_type = Ty;

public:
  StatusOr() = default;
  StatusOr(const Status &status) : base_type(status) {}
  StatusOr(Status && status) : base_type(std::move(status)) {}
  StatusOr(const value_type &value) : base_type(kOkStatus), my_value(value) {}
  StatusOr(value_type && value)
      : base_type(kOkStatus), my_value(std::move(value)) {}
  StatusOr(const Status &status, const value_type &value)
      : base_type(status), my_value(value) {}
  StatusOr(Status && status, value_type && value)
      : base_type(std::move(status)), my_value(std::move(value)) {}

  StatusOr(const StatusOr &that) : base_type(that), my_value(that.my_value) {}
  StatusOr(StatusOr && that) noexcept
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

	value_type value(this auto&& self) {
		contract_assert(self.ok());
		return self.my_value;
	}

	value_type value_or(this auto&& self, const value_type &default_value) {
		return self.ok() ? self.my_value : default_value;
	}

  explicit operator bool() const { return ok(); }

	value_type operator*(this auto&& self) { return self.my_value; }
	value_type operator->(this auto&& self) {
		contract_assert(self.ok());
		return self.my_value;
	}

private:
  value_type my_value;
};

nodiscard_msg(Status) LOXOGRAPH_API Status
    OkStatus(const std::source_location & = std::source_location::current());

nodiscard_msg(Status) LOXOGRAPH_API Status AlreadyExistsError(
    string_view,
    const std::source_location & = std::source_location::current());
nodiscard_msg(Status) LOXOGRAPH_API Status FileNotFoundError(
    string_view,
    const std::source_location & = std::source_location::current());
nodiscard_msg(Status) LOXOGRAPH_API Status UnknownError(
    string_view,
    const std::source_location & = std::source_location::current());
nodiscard_msg(Status) LOXOGRAPH_API Status PermissionDeniedError(
    string_view,
    const std::source_location & = std::source_location::current());
nodiscard_msg(Status) LOXOGRAPH_API Status InvalidArgument(
		string_view,
		const std::source_location & = std::source_location::current());
nodiscard_msg(Status) LOXOGRAPH_API Status CommandNotFound(
		string_view,
		const std::source_location & = std::source_location::current());
nodiscard_msg(Status) LOXOGRAPH_API Status EmptyInput(
		string_view,
		const std::source_location & = std::source_location::current());
nodiscard_msg(Status) LOXOGRAPH_API Status ParseError(
		string_view,
		const std::source_location & = std::source_location::current());
		
} // namespace net::ancillarycat::utils
