#include <format>
#include <string>
#include <string_view>

#include "config.hpp"
#include "status.hpp"
#include "fmt.hpp"
namespace net::ancillarycat::utils {
using namespace std::string_view_literals;
Status::Status(const Code code,
               const std::string_view message,
               const std::source_location &location)
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
  return utils::format("file {0}\n"
                     "              function {1},\n"
                     "              Ln {2} Col {3}\n",
                     my_location.file_name(),
                     my_location.function_name(),
                     my_location.line(),
                     my_location.column());
}
LOXOGRAPH_API
Status OkStatus(const std::source_location &location) {
  return {Status::kOkStatus, "OkStatus", location};
}
LOXOGRAPH_API
Status AlreadyExistsError(const std::string_view message,
                          const std::source_location &location) {
  return {Status::kAlreadyExistsError, message, location};
}
LOXOGRAPH_API
Status FileNotFoundError(const std::string_view message,
                         const std::source_location &location) {
  return {Status::kNotFoundError, message, location};
}
LOXOGRAPH_API
Status UnknownError(const std::string_view message,
                    const std::source_location &location) {
  return {Status::kUnknownError, message, location};
}
LOXOGRAPH_API
Status PermissionDeniedError(const std::string_view message,
                             const std::source_location &location) {
  return {Status::kPermissionDeniedError, message, location};
}
LOXO_NODISCARD_MSG(Status) LOXOGRAPH_API Status
    InvalidArgument(const string_view message,
                    const std::source_location &location) {
  return {Status::kInvalidArgument, message, location};
}
LOXO_NODISCARD_MSG(Status) LOXOGRAPH_API Status
    CommandNotFound(const string_view message,
                    const std::source_location &location) {
  return {Status::kCommandNotFound, message, location};
}
LOXO_NODISCARD_MSG(Status) LOXOGRAPH_API Status
    EmptyInput(const string_view message,
               const std::source_location &location) {
  return {Status::kEmptyInput, message, location};
}
LOXO_NODISCARD_MSG(Status) LOXOGRAPH_API Status
    ParseError(const string_view message,
               const std::source_location &location) {
  return {Status::kParseError, message, location};
}
} // namespace net::ancillarycat::utils
