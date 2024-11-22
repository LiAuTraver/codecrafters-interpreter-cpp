#pragma once

#include "config.hpp"

#ifndef LOXOGRAPH_USE_FMT_FORMAT
#  include <fmt.hh>
#endif
#include <std.hh>
namespace net::ancillarycat::utils {
/// @brief A class that represents the status of a function call. it's designed
/// to be as identical as possible to the `absl::Status` class, for
/// `absl::Status` seems to fail to compile with clang++ on Windows.
class Status;
/// @brief a simple file reader that reads the contents of a file
/// @note the file reader is not thread-safe, and will consume a lot of memory
/// if the file is too big. @todo here.
class file_reader;
using string = ::std::string;
using string_view = ::std::string_view;
using path = ::std::filesystem::path;
using ifstream = ::std::ifstream;
using ostringstream = ::std::ostringstream;
using namespace ::std::string_view_literals;
using namespace ::std::string_literals;
#ifndef LOXOGRAPH_USE_FMT_FORMAT
using ::std::format;
using ::std::print;
using ::std::println;
#else
using ::fmt::format;
using ::fmt::print;
using ::fmt::println;
#endif
} // namespace net::ancillarycat::utils
namespace net::ancillarycat::loxograph {
class lexer;
class lex_error;

class Token;
class TokenType;

class Expr;
class Literal;
class Unary;
class Binary;
class Grouping;

class ExprVisitor;
class DummyVisitor;

class Parser;

using utils::operator""s;
using utils::operator""sv;
static constexpr auto tolerable_chars = "_`"sv;
/// @note intolarable in codecrafter test
static constexpr auto conditional_tolerable_chars = "@$#"sv;
static constexpr auto whitespace_chars = " \t\r"sv;
static constexpr auto newline_chars = "\n\v\f"sv;
} // namespace net::ancillarycat::loxograph
