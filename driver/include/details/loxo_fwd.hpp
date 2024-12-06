#pragma once
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>

#include <net/ancillarycat/utils/config.hpp>
#include <net/ancillarycat/utils/format.hpp>

#ifdef LIBLOXO_SHARED
#  ifdef _WIN32
#    ifdef driver_EXPORTS
#      define LOXO_API __declspec(dllexport)
#    else
#      define LOXO_API __declspec(dllimport)
#    endif
#  else
#    define LOXO_API __attribute__((visibility("default")))
#  endif
#  define LOXO_INLINE
#else
#  define LOXO_API
#  define LOXO_INLINE inline
#endif


// NOLINTBEGIN(bugprone-forward-declaration-namespace)
namespace net::ancillarycat::loxo {

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
class Logical;
class Call;
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
class While;
class For;
class Function;
class Return;
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
class Callable;

class ScopeAssoc;
} // namespace evaluation

using utils::operator""s;
using utils::operator""sv;
inline static constexpr auto tolerable_chars = "_`"sv;
/// @note intolarable in codecrafter test
// inline static constexpr auto conditional_tolerable_chars = "@$#"sv;
inline static constexpr auto whitespace_chars = " \t\r"sv;
inline static constexpr auto newline_chars = "\n\v\f"sv;
} // namespace net::ancillarycat::loxo
// NOLINTEND(bugprone-forward-declaration-namespace)
