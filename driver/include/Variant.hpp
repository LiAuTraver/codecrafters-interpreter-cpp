#pragma once

#include <utility>
#include <variant>
#include <string>
#include <string_view>
#include <type_traits>
#include <typeinfo>

#include "config.hpp"
#include "loxo_fwd.hpp"
#include "fmt.hpp"

namespace net::ancillarycat::utils {
template <typename... Types> class Variant : public Printable, Viewable {
public:
  using variant_type = std::variant<Types...>;

public:
  inline constexpr Variant() = default;

  template <typename Ty>
    requires(!std::same_as<std::decay_t<Ty>, Variant>)
  Variant(Ty &&value) : my_variant(std::forward<Ty>(value)) {}

  Variant(const Variant &) = default;
  Variant(Variant &&) noexcept = default;
  Variant &operator=(const Variant &) = default;
  Variant &operator=(Variant &&) noexcept = default;
  virtual ~Variant() override = default;

public:
  template <typename Callable>
  auto visit(this auto &&self, Callable &&callable) -> decltype(auto) {
    using ReturnType = decltype(std::forward<Callable>(callable)(
        std::declval<variant_type>()));
    return self.is_valid()
               ? std::visit(std::forward<Callable>(callable), self.my_variant)
               : ReturnType{};
  }
  string_view_type type_name() const {
    return is_valid() ? this->visit([]([[maybe_unused]] const auto &value)
                                        -> string_view_type {
      return typeid(value).name();
    })
                      : "invalid state"sv;
  }
  auto index() const noexcept { return my_variant.index(); }
  template <typename... Args>
    requires requires {
      std::declval<variant_type>().template emplace<Args...>(
          std::declval<Args>()...);
    }
  auto emplace(Args &&...args) -> decltype(auto) {
    return my_variant.template emplace<Args...>(std::forward<Args>(args)...);
  }
  template <typename Args>
    requires requires { std::declval<variant_type>().template emplace<Args>(); }
  constexpr auto emplace() -> decltype(auto) {
    return my_variant.template emplace<Args>();
  }
  constexpr auto &get() const { return my_variant; }
  constexpr auto swap(Variant &that) noexcept(
      std::conjunction_v<std::is_nothrow_move_constructible<Types...>,
                         std::is_nothrow_swappable<Types...>>) -> Variant & {
    my_variant.swap(that.my_variant);
    return *this;
  }
  string_type underlying_string(
      const FormatPolicy &format_policy = FormatPolicy::kDefault) const {
    return this->visit([&](const auto &value) -> string_type {
      return value.to_string(format_policy);
    });
  }

private:
  variant_type my_variant{Monostate{}};

private:
  inline constexpr bool is_valid() const noexcept {
    auto ans = my_variant.index() != std::variant_npos;
    contract_assert(ans);
    return ans;
  }

private:
  constexpr auto to_string_impl(const FormatPolicy &format_policy) const
      -> string_type override {
    return typeid(decltype(*this)).name();
  }
  constexpr auto to_string_view_impl(const FormatPolicy &format_policy) const
      -> string_view_type override {
    return typeid(decltype(*this)).name();
  }

private:
  /// @brief get the value of the variant; a wrapper around @link std::get_if
  /// @endlink
  template <class Ty, class... MyTypes>
  friend inline constexpr auto get_if(Variant<MyTypes...> *v) noexcept;
  /// @brief get the value of the variant; a wrapper around @link std::get
  /// @endlink
  template <class Ty, class... MyTypes>
  friend inline constexpr auto get(const Variant<MyTypes...> &v)
      -> decltype(auto);
};
/// @brief check if the variant holds a specific type; a wrapper around @link
/// std::holds_alternative @endlink
template <class Ty, class... MyTypes>
inline constexpr bool holds_alternative(const Variant<MyTypes...> &v) noexcept {
  return std::holds_alternative<Ty>(v.get());
}
template <class Ty, class... MyTypes>
inline constexpr auto get(const Variant<MyTypes...> &v) -> decltype(auto) {
  return v.is_valid() ? std::get<Ty>(v.get()) : Ty{};
}
template <class Ty, class... MyTypes>
LOXO_NODISCARD_MSG(get_if) inline constexpr auto get_if(Variant<MyTypes...> *v) noexcept {
  return v->is_valid() ? std::get_if<Ty>(&v->get()) : nullptr;
}
} // namespace net::ancillarycat::utils
