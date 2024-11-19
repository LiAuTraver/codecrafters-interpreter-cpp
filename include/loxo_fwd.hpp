#pragma once

#include <filesystem>
#include <iosfwd>
#include <string>
#include <string_view>
#if (defined(__clang__) && defined(_MSC_VER)) || (!__has_include(<fmt/core.h>))
#include <format>
#include <print>
#else
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/os.h>
#include <fmt/ostream.h>
#endif
namespace net::ancillarycat::utils {
class Status;
/// @brief a simple file reader that reads the contents of a file
/// @tparam PathType the path type
/// @tparam StringType the string type
/// @tparam InputStreamType the input stream type
/// @tparam OutputStringStreamType the output string stream type
/// @note the file reader is not thread-safe, and will consume a lot of memory
/// if the file is too big. @todo here.
template <typename PathType = std::filesystem::path,
          typename StringType = std::string,
          typename InputStreamType = std::ifstream,
          typename OutputStringStreamType = std::ostringstream>
class file_reader;
using StringType = std::string;
using StringViewType = std::string_view;
using PathType = std::filesystem::path;
using BooleanType = bool;
using StatusType = Status;
using InputStreamType = std::ifstream;
using OutputStringStreamType = std::ostringstream;
using PathType = std::filesystem::path;
} // namespace net::ancillarycat::utils
namespace net::ancillarycat::loxograph {
class lexer;
class Token;
class TokenType;
using ::net::ancillarycat::utils::BooleanType;
using ::net::ancillarycat::utils::file_reader;
using ::net::ancillarycat::utils::InputStreamType;
using ::net::ancillarycat::utils::OutputStringStreamType;
using ::net::ancillarycat::utils::PathType;
using ::net::ancillarycat::utils::Status;
using ::net::ancillarycat::utils::StatusType;
using ::net::ancillarycat::utils::StringType;
using ::net::ancillarycat::utils::StringViewType;
using namespace std::string_view_literals;
static constexpr auto tolerable_chars = "_$@`"sv;
static constexpr auto whitespace_chars = " \t\r"sv;
static constexpr auto newline_chars = "\n\v\f"sv;
/// @note use fmt::print, fmt::println when compiling with clang-cl.exe will
/// cause some wired error: Critical error detected c0000374
/// A breakpoint instruction (__debugbreak() statement or a similar call) was
/// executed, which related to heap corruption. The program will terminate.
#if (defined(__clang__) && defined(_MSC_VER)) || (!__has_include(<fmt/core.h>))
using ::std::format;
using ::std::print;
using ::std::println;
#else
using ::fmt::format;
using ::fmt::print;
using ::fmt::println;
#endif
} // namespace net::ancillarycat::loxograph
