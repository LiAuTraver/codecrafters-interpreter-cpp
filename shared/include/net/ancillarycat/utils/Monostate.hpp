#pragma once
#include <compare>
#include <string>
#include <string_view>

#include "config.hpp"
#include "format.hpp"

namespace net::ancillarycat::utils {
/// @brief Represents a stateless type that can be used as a placeholder
/// @implements Printable, Viewable
/// @remark Different from `std::monostate`, this class implements two traits
/// which can be extremely important in my @link Variant @endlink class.
class Monostate final : public Printable, public Viewable {
public:
  inline consteval Monostate() = default;
  inline virtual ~Monostate() override = default;
  inline constexpr Monostate(const Monostate &) {}
  inline constexpr Monostate(Monostate &&) noexcept {}
  inline constexpr auto operator=(const Monostate &) -> Monostate & {
    return *this;
  }
  inline constexpr auto operator=(Monostate &&) noexcept -> Monostate & {
    return *this;
  }

private:
  inline constexpr auto
  to_string_impl(const FormatPolicy &) const -> string_type override {
    return {};
  }
  inline constexpr auto
  to_string_view_impl(const FormatPolicy &) const -> string_view_type override {
    return {};
  }
  friend inline consteval auto operator==(const Monostate &,
                                          const Monostate &) noexcept -> bool {
    return true;
  }
  friend inline consteval auto
  operator<=>(const Monostate &,
              const Monostate &) noexcept -> std::strong_ordering {
    return std::strong_ordering::equal;
  }
};
} // namespace net::ancillarycat::utils
