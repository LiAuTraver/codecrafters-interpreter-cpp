/// @note no include guard.
#ifdef AC_LOXO_SCOPEASSOC_INL
#  error                                                                       \
      "please do not include ScopeAssoc.inl in other files; include Environment.hpp instead"
#endif
#define AC_LOXO_SCOPEASSOC_INL
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <unordered_map>
#include <string>
#include <string_view>
#include <variant>
#include <accat/auxilia/auxilia.hpp>

#include "details/loxo_fwd.hpp"
#include "details/IVisitor.hpp"

namespace accat::loxo::evaluation {
class ScopeAssoc : public auxilia::Printable {
  friend class ::accat::loxo::Environment;

public:
  using variant_type = IVisitor::variant_type;
  using string_view_type = IVisitor::string_view_type;
  using association_t =
      std::pair<string_type, std::pair<variant_type, uint_least32_t>>;
  using associations_t =
      std::unordered_map<string_type, std::pair<variant_type, uint_least32_t>>;

public:
  constexpr ScopeAssoc() = default;
  ~ScopeAssoc() = default;
  ScopeAssoc(const ScopeAssoc &that) = default;
  ScopeAssoc(ScopeAssoc &&that) noexcept = default;
  auto operator=(const ScopeAssoc &that) -> ScopeAssoc & = default;
  auto operator=(ScopeAssoc &&that) noexcept -> ScopeAssoc & = default;

private:
  auto add(const string_type &,
           const variant_type &,
           uint_least32_t = std::numeric_limits<uint_least32_t>::quiet_NaN())
      -> auxilia::Status;
  auto find(this auto &&self, const string_type &name)
      -> std::optional<associations_t::iterator>;

private:
  associations_t associations{};

public:
  auto to_string(const auxilia::FormatPolicy &) const -> string_type;
};
auto ScopeAssoc::find(this auto &&self, const string_type &name)
    -> std::optional<associations_t::iterator> {
  if (auto it = self.associations.find(name); it != self.associations.end())
    return it;
  return std::nullopt;
}
} // namespace accat::loxo::evaluation
