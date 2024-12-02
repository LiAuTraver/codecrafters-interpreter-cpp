#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>

#include "config.hpp"
#include "utils.hpp"
#include "loxo_fwd.hpp"

namespace net::ancillarycat::loxograph {
/// @brief enhanced token type, more like rust's enum
/// @implements utils::Printable
/// @implements utils::Viewable
class LOXOGRAPH_API TokenType : public utils::Printable,
                                public utils::Viewable {
public:
  enum type_t : uint16_t;

public:
  enum type_t : uint16_t {
    // clang-format off
    kMonostate = 0,
    // Single-character tokens.
    kLeftParen, kRightParen, kLeftBrace, kRightBrace, kComma, kDot, kMinus, kPlus, kSemicolon, kSlash, kStar,
    // One or two character tokens.
    kBang, kBangEqual, kEqual, kEqualEqual, kGreater, kGreaterEqual, kLess, kLessEqual,
    // Literals.
    kIdentifier, kString, kNumber,
    // Keywords.
    kAnd, kClass, kElse, kFalse, kFun, kFor, kIf, kNil, kOr, kPrint, kReturn, kSuper, kThis, kTrue, kVar, kWhile,
    // lex error.
    kLexError,
    // end of file.
    kEndOfFile,
    // clang-format on
  };

public:
  /// @note no-explicit
  TokenType(const enum type_t type = kMonostate)
      : type(type) {} // NOLINT(google-explicit-constructor)
  /// @brief for @link fmt::format @endlink
  friend auto format_as(const TokenType &) noexcept -> string_view_type;
  /// @note member function otherwise `ambiguous call` since the ctor is not explicit
	auto operator<=>(this auto&& self, const type_t &token_t) {
		return self.type <=> token_t;
	}
  friend auto operator==(const TokenType &wrapped_token_t,
                         const type_t &token_t) {
    return wrapped_token_t.type == token_t;
  }
  friend auto operator!=(const TokenType &wrapped_token_t,
                         const type_t &token_t) {
    return wrapped_token_t.type != token_t;
  }
  friend auto operator<=>(const type_t &token_t,
                          const TokenType &wrapped_token_t) {
    return token_t <=> wrapped_token_t.type;
  }
  friend auto operator==(const type_t &token_t,
                         const TokenType &wrapped_token_t) {
    return token_t == wrapped_token_t.type;
  }
  friend auto operator!=(const type_t &token_t,
                         const TokenType &wrapped_token_t) {
    return token_t != wrapped_token_t.type;
  }
  friend auto operator<=>(const TokenType &lhs, const TokenType &rhs) {
    return lhs.type <=> rhs.type;
  }
  friend auto operator==(const TokenType &lhs, const TokenType &rhs) {
    return lhs.type == rhs.type;
  }
  friend auto operator!=(const TokenType &lhs, const TokenType &rhs) {
    return lhs.type != rhs.type;
  }

public:
  type_t type;

private:
  auto to_string_impl(const utils::FormatPolicy &) const
      -> string_type override;
  auto to_string_view_impl(const utils::FormatPolicy &) const
      -> string_view_type override;
};
inline static const auto keywords =
    std::unordered_map<TokenType::string_view_type, TokenType>{
        {"and"sv, {TokenType::kAnd}},
        {"class"sv, {TokenType::kClass}},
        {"else"sv, {TokenType::kElse}},
        {"false"sv, {TokenType::kFalse}},
        {"for"sv, {TokenType::kFor}},
        {"fun"sv, {TokenType::kFun}},
        {"if"sv, {TokenType::kIf}},
        {"nil"sv, {TokenType::kNil}},
        {"or"sv, {TokenType::kOr}},
        {"print"sv, {TokenType::kPrint}},
        {"return"sv, {TokenType::kReturn}},
        {"super"sv, {TokenType::kSuper}},
        {"this"sv, {TokenType::kThis}},
        {"true"sv, {TokenType::kTrue}},
        {"var"sv, {TokenType::kVar}},
        {"while"sv, {TokenType::kWhile}},
    };
} // namespace net::ancillarycat::loxograph
