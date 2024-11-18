#include "config.hpp"
#include "loxo_fwd.hpp"
#include <format>
#include <source_location>
#include <stacktrace>
#include <string>
#include <string_view>


namespace net::ancillarycat::utils {
class nodiscard_msg(Status) Status {
public:
  enum nodiscard_msg(Code) Code {
    kOkStatus = 0,
    kAlreadyExistsError = 1,
    kNotFoundError = 2,
    kUnknownError = 3,
  };

  nodiscard_msg(Status)
      Status(Code code, std::string_view message = "<no message provided>",
             std::source_location location = std::source_location::current())
      : my_code(code), my_message(message), my_location(location) {}

  nodiscard_msg(Status) bool ok() const { return my_code == kOkStatus; }
  Code code() const { return my_code; }
  std::string_view message() const { return my_message; }
  std::source_location location() const { return my_location; }
  std::string stacktrace() const {
    return std::format("Stacktrace:\n{}", std::stacktrace::current());
  }

  inline constexpr bool ignore_error() const /*noexcept*/ {
    contract_assert(ok()) return true;
  }

  inline std::string from_source_location() const {
    return std::format("file {0}\n"
                       "              function {1},\n"
                       "              Ln {2} Col {3}\n",
                       my_location.file_name(), my_location.function_name(),
                       my_location.line(), my_location.column());
  }

private:
  Code my_code;
  std::string_view my_message;
  std::source_location my_location;
};

static Status OkStatus(
    const std::source_location &location = std::source_location::current()) {
  return Status(Status::kOkStatus, "OkStatus", location);
}
static Status AlreadyExistsError(
    std::string_view message,
    const std::source_location &location = std::source_location::current()) {
  return Status(Status::kAlreadyExistsError, message, location);
}
static Status NotFoundError(
    std::string_view message,
    const std::source_location &location = std::source_location::current()) {
  return Status(Status::kNotFoundError, message, location);
}
static Status UnknownError(
    std::string_view message,
    const std::source_location &location = std::source_location::current()) {
  return Status(Status::kUnknownError, message, location);
}

} // namespace net::ancillarycat::utils