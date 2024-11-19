#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include "loxo_fwd.hpp"
namespace net::ancillarycat::loxograph {
class TokenType {
public:
  enum type_t : uint16_t;
  type_t type;

  enum type_t : uint16_t {
    kMonostate = 0,
    // Single-character tokens.
    kLeftParen,
    kRightParen,
    kLeftBrace,
    kRightBrace,
    kComma,
    kDot,
    kMinus,
    kPlus,
    kSemicolon,
    kSlash,
    kStar,

    // One or two character tokens.
    kBang,
    kBangEqual,
    kEqual,
    kEqualEqual,
    kGreater,
    kGreaterEqual,
    kLess,
    kLessEqual,

    // Literals.
    kIdentifier,
    kString,
    kNumber,

    // Keywords.
    kAnd,
    kClass,
    kElse,
    kFalse,
    kFun,
    kFor,
    kIf,
    kNil,
    kOr,
    kPrint,
    kReturn,
    kSuper,
    kThis,
    kTrue,
    kVar,
    kWhile,

    // lex error,
    kLexError,
    // end of file
    kEndOfFile,
  };

public:
  using string_t = StringType;
  using string_view_t = StringViewType;

public:
  /// @note no-explicit
  inline TokenType(enum type_t type = kMonostate) : type(type) {}

public:
  string_view_t to_string_view() const noexcept;
  string_t to_string() const noexcept;
};
} // namespace net::ancillarycat::loxograph
