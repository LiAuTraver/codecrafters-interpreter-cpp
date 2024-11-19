#pragma once
#include <format>
#include <source_location>
#if __has_include(<stacktrace>)
#include <stacktrace>
#endif
#include <string>
#include <string_view>
#include "config.hpp"
#include "loxo_fwd.hpp"

namespace net::ancillarycat::utils {
class nodiscard_msg(Status) Status {
public:
  enum nodiscard_msg(Code) Code : uint8_t {
    kOkStatus = 0,
    kError = 1,
    kAlreadyExistsError = 2,
    kNotFoundError = 3,
    kMovedFrom = 4,
    kPermissionDeniedError = 5,
    kUnknownError = std::numeric_limits<uint8_t>::max()
  };
  nodiscard_msg(Status) Status()
      : my_code(kOkStatus), my_message("<default initialized>"),
        my_location(std::source_location::current()) {}
  nodiscard_msg(Status)
      Status(Code, std::string_view = "<no message provided>",
             std::source_location = std::source_location::current());
  nodiscard_msg(Status) Status(Status &&) noexcept;
  nodiscard_msg(Status) Status(const Status &);
	Status &operator=(const Status & that) {
		my_code = that.my_code;
		my_message = that.my_message;
		my_location = that.my_location;
		return *this;
	}
	Status &operator=(Status && that) noexcept{
		my_code = that.my_code;
		my_message = std::move(that.my_message);
		my_location = that.my_location;
		that.my_code = kMovedFrom;
		that.my_message = "This status has been moved from.";
		that.my_location = std::source_location::current();
		return *this;
	}

  nodiscard_msg(Status) bool ok() const;
  Code code() const;
  std::string_view message() const;
  std::source_location location() const;
  std::string stacktrace() const;
  inline constexpr bool ignore_error() const /*noexcept*/;
  inline std::string from_source_location() const;

private:
  Code my_code;
  std::string my_message;
  std::source_location my_location;
};

Status OkStatus(const std::source_location & = std::source_location::current());
Status AlreadyExistsError(
    std::string_view,
    const std::source_location & = std::source_location::current());
Status FileNotFoundError(std::string_view, const std::source_location & =
                                               std::source_location::current());
Status
UnknownError(std::string_view,
             const std::source_location & = std::source_location::current());
Status PermissionDeniedError(
    std::string_view,
    const std::source_location & = std::source_location::current());
} // namespace net::ancillarycat::utils