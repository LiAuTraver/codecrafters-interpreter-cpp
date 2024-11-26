#pragma once
#include <any>
#include <cstdint>
#include <ostream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>

#include "config.hpp"
#include "loxo_fwd.hpp"

#ifdef LOXOGRAPH_USE_FMT_FORMAT
#  include <fmt/core.h>
#  include <fmt/format.h>
#else
#  include <format>
#  include <print>
#endif

namespace net::ancillarycat::utils {

enum FormatPolicy : uint8_t;
enum FormatPolicy : uint8_t {
  kDefault = 0,
  kTokenOnly = 1,
};
/// @interface Printable
/// @brief A class that represents a printable object
class Printable {
  friend auto operator<<(std::ostream &os, const Printable &p)
      -> std::ostream & {
    return os << p.to_string();
  }

public:
  using string_type = std::string;

public:
  constexpr Printable() = default;
  virtual ~Printable() = default;

public:
  auto to_string(const FormatPolicy &format_policy = kDefault) const
      -> string_type {
    return to_string_impl(format_policy);
  }

private:
  virtual auto to_string_impl(const FormatPolicy &format_policy) const
      -> string_type = 0;
  friend auto format_as(const Printable &p,
                        const FormatPolicy &format_policy = kDefault)
      -> string_type {
    return p.to_string(format_policy);
  }
};

class Viewable {
public:
  using string_view_type = std::string_view;

public:
  constexpr Viewable() = default;
  virtual ~Viewable() = default;

public:
  auto to_string_view(const FormatPolicy &format_policy = kDefault) const {
    return to_string_view_impl(format_policy);
  }

private:
  virtual auto to_string_view_impl(const FormatPolicy &format_policy) const
      -> string_view_type = 0;
};
#ifndef LOXOGRAPH_USE_FMT_FORMAT
#  include <format>
#  include <print>
using ::std::format;
using ::std::print;
using ::std::println;
#else
using ::fmt::format;
using ::fmt::print;
using ::fmt::println;
#endif
// clang-format off
/*!
   @brief cast the literal to the specified type and also log the value in debug mode.
   @note clang-cl.exe does not fully support `try` and `catch` blocks but
   `any_cast` will throw an exception if the cast fails. For more information, see
   <a href="https://clang.llvm.org/docs/MSVCCompatibility.html#asynchronous-exceptions">Asynchronous Exceptions</a>,
   <a href="https://stackoverflow.com/questions/7049502/c-try-and-try-catch-finally">try-catch-finally</a>,
   and <a href="https://learn.microsoft.com/en-us/cpp/cpp/try-except-statement">try-except-statement</a>.
 */
// clang-format on
template <typename Ty>
inline auto get_if(const std::any &literal) -> decltype(auto)
  requires std::default_initializable<Ty>
{
  const auto ptr = std::any_cast<Ty>(&literal);
  if (not ptr)
    dbg_block({
      dbg(error, "bad any cast.");
      dbg(info,
          "Expect type: {}, actual type: {}",
          typeid(Ty).name(),
          literal.type().name());
      dbg(warn,
          "\033[033mNote: this lexer treat all number as long double; meybe "
          "you "
          "accidentally passed an integer?\033[0m");
      // contract_assert(0);
    });

  // dbg(info, "literal type: {}, value: {}", typeid(Ty).name(), *ptr);
  return ptr;
}
template <typename Ty>
  requires std::is_arithmetic_v<std::remove_cvref_t<Ty>>
bool is_integer(Ty &&value) noexcept {
  return std::trunc(std::forward<Ty>(value)) == value;
}
} // namespace net::ancillarycat::utils
#ifdef LOXOGRAPH_USE_FMT_FORMAT
/// @note no need if we have `format_as` function
// template <> struct fmt::formatter<net::ancillarycat::utils::Printable> {
//   constexpr auto parse(::fmt::format_parse_context &ctx) { return
//   ctx.begin(); } template <typename FormatContext> auto format(const
//   net::ancillarycat::utils::Printable &p,
//               FormatContext &ctx) const{
//     return ::fmt::format_to(
//         ctx.out(),
//         "{}",
//         p.to_string(net::ancillarycat::utils::FormatPolicy::kDefault));
//   }
// };
#else
template <> struct std::formatter<net::ancillarycat::utils::Printable> {
  constexpr auto parse(::std::format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  auto format(const net::ancillarycat::utils::Printable &p,
              FormatContext &ctx) {
    return ::std::format_to(
        ctx.out(),
        "{}",
        p.to_string(net::ancillarycat::utils::FormatPolicy::kDefault));
  }
};
#endif
