#include <format>
#include <string>
#include <string_view>

#include "config.hpp"
#include "status.hpp"
namespace net::ancillarycat::utils {
using namespace std::string_view_literals;
Status::Status(Code code,
               std::string_view message,
               std::source_location location)
    : my_code(code), my_message(message), my_location(location) {}
Status::Status(Status &&that) noexcept
    : my_code(that.my_code), my_message(std::move(that.my_message)),
      my_location(that.my_location) {
  that.my_code = kMovedFrom;
  that.my_message = "This status has been moved from."sv;
  that.my_location = std::source_location::current();
}
Status::Status(const Status &that) {
  my_code = that.my_code;
  my_message = that.my_message;
  my_location = that.my_location;
}
Status &Status::operator=(const Status &that) {
  my_code = that.my_code;
  my_message = that.my_message;
  my_location = that.my_location;
  return *this;
}
Status &Status::operator=(Status &&that) noexcept {
  my_code = that.my_code;
  my_message = std::move(that.my_message);
  my_location = that.my_location;
  that.my_code = kMovedFrom;
  that.my_message = "This status has been moved from.";
  that.my_location = std::source_location::current();
  return *this;
}
bool Status::ok() const { return my_code == kOkStatus; }
Status::Code Status::code() const { return my_code; }
std::string_view Status::message() const { return my_message; }
std::source_location Status::location() const { return my_location; }
std::string Status::stacktrace() const { return LOXOGRAPH_STACKTRACE; }
void Status::ignore_error() const { contract_assert(ok()); }
std::string Status::from_source_location() const {
  return std::format("file {0}\n"
                     "              function {1},\n"
                     "              Ln {2} Col {3}\n",
                     my_location.file_name(),
                     my_location.function_name(),
                     my_location.line(),
                     my_location.column());
}
LOXOGRAPH_API
Status OkStatus(const std::source_location &location) {
  return Status(Status::kOkStatus, "OkStatus", location);
}
LOXOGRAPH_API
Status AlreadyExistsError(std::string_view message,
                          const std::source_location &location) {
  return Status(Status::kAlreadyExistsError, message, location);
}
LOXOGRAPH_API
Status FileNotFoundError(std::string_view message,
                         const std::source_location &location) {
  return Status(Status::kNotFoundError, message, location);
}
LOXOGRAPH_API
Status UnknownError(std::string_view message,
                    const std::source_location &location) {
  return Status(Status::kUnknownError, message, location);
}
LOXOGRAPH_API
Status PermissionDeniedError(std::string_view message,
                             const std::source_location &location) {
  return Status(Status::kPermissionDeniedError, message, location);
}
} // namespace net::ancillarycat::utils
