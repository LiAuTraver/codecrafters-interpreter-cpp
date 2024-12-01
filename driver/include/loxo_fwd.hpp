#pragma once

#include <any>
#include <cmath>
#include <filesystem>
#include <source_location>
#include <string>
#include <string_view>

#include "config.hpp"
namespace net::ancillarycat::utils {
/// @brief A simple variant wrapper class for convenience when evaluating
/// expressions, especially when the operation was `to_string` or check the
/// type's name when debugging.
/// @note exception-free variant wrapper
template <typename... Types> class Variant;
/// @brief A class that represents the status of a function call. it's designed
/// to be as identical as possible to the `absl::Status` class, for
/// `absl::Status` seems to fail to compile with clang++ on Windows.
class Status;
/// @brief represents a value that can be stored in a @link StatusOr object
/// @tparam Ty the type of the value
/// @remarks similiar to Microsoft's @link std::_SMF_control class, which was
/// used in @link std::optional
template <typename Ty>
concept Storable = std::conjunction_v<std::is_default_constructible<Ty>,
                                      std::is_nothrow_destructible<Ty>,
                                      std::is_nothrow_constructible<Ty>>;
/// @brief A class that represents the status of a function call, or a
/// value.
///         it's designed to be as identical as possible to the
///         `absl::StatusOr` class.
/// @tparam Ty the type of the value
template <Storable Ty> class StatusOr;
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
} // namespace net::ancillarycat::utils
// NOLINTBEGIN(bugprone-forward-declaration-namespace)
namespace net::ancillarycat::loxograph {

class lexer;
class lex_error;

class Token;
class TokenType;

class parser;
class parse_error;

namespace expression {
class Expr;
class Literal;
class Unary;
class Binary;
class Grouping;
class Variable;
class IllegalExpr;

class ExprVisitor;
class DummyVisitor;
} // namespace expression
namespace statement {
class Stmt;
class Variable;
class Print;
class Expression;

class StmtVisitor;
class DummyVisitor;
} // namespace statement
namespace evaluation {
class Evaluatable;
class Number;
class String;
class Value;
class Boolean;
class Nil;
class Error;
} // namespace evaluation

class interpreter;
using utils::operator""s;
using utils::operator""sv;
inline static constexpr auto tolerable_chars = "_`"sv;
/// @note intolarable in codecrafter test
inline static constexpr auto conditional_tolerable_chars = "@$#"sv;
inline static constexpr auto whitespace_chars = " \t\r"sv;
inline static constexpr auto newline_chars = "\n\v\f"sv;
} // namespace net::ancillarycat::loxograph
// NOLINTEND(bugprone-forward-declaration-namespace)
