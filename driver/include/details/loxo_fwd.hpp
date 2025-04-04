#pragma once
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>

#include <accat/auxilia/auxilia.hpp>

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
#else
#  define LOXO_API
#endif

namespace accat::loxo {

class Token;
class TokenType;

class lexer;
class lex_error;

class parser;
class parse_error;

class interpreter;
class Environment;

class Resolver;
// NOLINTBEGIN(bugprone-forward-declaration-namespace)
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
class Get;
class Set;
class IllegalExpr;

class ExprVisitor;
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
class Class;
class Return;
class IllegalStmt;

class StmtVisitor;
} // namespace statement
namespace evaluation {
class Evaluatable;
class Number;
class String;
class Value;
class Boolean;
class Nil;
class Function;
class Class;
class Instance;

class ScopeAssoc;
} // namespace evaluation
// NOLINTEND(bugprone-forward-declaration-namespace)

using auxilia::operator""s;
using auxilia::operator""sv;
inline static constexpr auto tolerable_chars = "_`"sv;
/// @note intolarable in codecrafter test
// inline static constexpr auto conditional_tolerable_chars = "@$#"sv;
inline static constexpr auto whitespace_chars = " \t\r"sv;
inline static constexpr auto newline_chars = "\n\v\f"sv;
} // namespace accat::loxo
