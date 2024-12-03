#pragma once
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>

#include "config.hpp"

namespace net::ancillarycat::utils {
class Monostate;
/// @brief a concept that checks if the types are variantable for my custom
/// @link Variant @endlink class,
/// where the first type must be @link Monostate @endlink
/// @tparam Types the types to check
template <typename... Types>
concept Variantable = requires {
  std::is_same_v<std::tuple_element_t<0, std::tuple<Types...>>, Monostate>;
};
/// @brief represents a value that can be stored in a
/// @link StatusOr @endlink object
/// @tparam Ty the type of the value
/// @remarks similiar to Microsoft's @link std::_SMF_control @endlink class,
/// which was used in @link std::optional @endlink
template <typename Ty>
concept Storable = std::conjunction_v<std::is_default_constructible<Ty>,
                                      std::is_nothrow_destructible<Ty>,
                                      std::is_nothrow_constructible<Ty>>;

template <Variantable... Types> class Variant;
class Status;
template <Storable Ty> class StatusOr;
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

class Token;
class TokenType;

class lexer;
class lex_error;

class parser;
class parse_error;

class interpreter;
class Environment;

namespace expression {
class Expr;
class Literal;
class Unary;
class Binary;
class Grouping;
class Variable;
class Assignment;
class IllegalExpr;

class ExprVisitor;
class DummyVisitor;
} // namespace expression
namespace statement {
class Stmt;
class Variable;
class Print;
class Expression;
class Block;
class If;
class IllegalStmt;

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

class ScopeAssoc;
} // namespace evaluation

using utils::operator""s;
using utils::operator""sv;
inline static constexpr auto tolerable_chars = "_`"sv;
/// @note intolarable in codecrafter test
// inline static constexpr auto conditional_tolerable_chars = "@$#"sv;
inline static constexpr auto whitespace_chars = " \t\r"sv;
inline static constexpr auto newline_chars = "\n\v\f"sv;
} // namespace net::ancillarycat::loxograph
// NOLINTEND(bugprone-forward-declaration-namespace)
