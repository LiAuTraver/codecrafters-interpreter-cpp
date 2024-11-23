#pragma once
#include <unordered_map>
#include <string>
#include <string_view>
#include <cstdint>

#include "config.hpp"
#include "loxo_fwd.hpp"

namespace net::ancillarycat::loxograph {
class LOXOGRAPH_API TokenType {
public:
  enum type_t : uint16_t;
  using string_t = utils::string;
  using string_view_t = utils::string_view;

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
  TokenType(enum type_t type = kMonostate) : type(type) {} // NOLINT(google-explicit-constructor)

public:
  string_view_t to_string_view() const noexcept;
  string_t to_string() const noexcept;
  /// @brief for @ref fmt::format
  friend auto format_as(TokenType) noexcept -> string_view_t;

public:
  type_t type;
};
inline static const auto keywords =
    std::unordered_map<TokenType::string_view_t, TokenType>{
        {"and"sv, {TokenType::kAnd}},       {"class"sv, {TokenType::kClass}},
        {"else"sv, {TokenType::kElse}},     {"false"sv, {TokenType::kFalse}},
        {"for"sv, {TokenType::kFor}},       {"fun"sv, {TokenType::kFun}},
        {"if"sv, {TokenType::kIf}},         {"nil"sv, {TokenType::kNil}},
        {"or"sv, {TokenType::kOr}},         {"print"sv, {TokenType::kPrint}},
        {"return"sv, {TokenType::kReturn}}, {"super"sv, {TokenType::kSuper}},
        {"this"sv, {TokenType::kThis}},     {"true"sv, {TokenType::kTrue}},
        {"var"sv, {TokenType::kVar}},       {"while"sv, {TokenType::kWhile}},
    };
} // namespace net::ancillarycat::loxograph

/// @brief for @ref std::format
template <>
struct std::formatter<net::ancillarycat::loxograph::TokenType>
    : std::formatter<net::ancillarycat::loxograph::TokenType::string_view_t> {
  auto format(net::ancillarycat::loxograph::TokenType t,
              std::format_context &ctx) const -> decltype(ctx.out());
};
