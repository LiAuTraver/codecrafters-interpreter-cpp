/// @note no include guard.
#ifdef AC_LOX_SCOPEASSOC_INL
#  error                                                                       \
      "please do not include ScopeAssoc.inl in other files; include Environment.hpp instead"
#endif
#define AC_LOX_SCOPEASSOC_INL
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

#include "details/lox_fwd.hpp"
#include "details/IVisitor.hpp"

namespace accat::lox::evaluation {
class ScopeAssoc : public auxilia::Printable {
  friend class ::accat::lox::Environment;

public:
  using variant_type = IVisitor::variant_type;
  using string_view_type = IVisitor::string_view_type;
  using association_t =
      std::pair<string_type, std::pair<variant_type, uint_least32_t>>;
  using associations_t =
      std::unordered_map<string_type, std::pair<variant_type, uint_least32_t>>;

public:
  ScopeAssoc() = default;
  ~ScopeAssoc() = default;
  ScopeAssoc(const ScopeAssoc &that) = default;
  ScopeAssoc(ScopeAssoc &&that) noexcept = default;
  auto operator=(const ScopeAssoc &that) -> ScopeAssoc & = default;
  auto operator=(ScopeAssoc &&that) noexcept -> ScopeAssoc & = default;

private:
  auto add(string_view_type,
           const variant_type &,
           uint_least32_t = std::numeric_limits<uint_least32_t>::quiet_NaN())
      -> auxilia::Status;
  auto find(string_view_type) -> std::optional<associations_t::iterator>;

  auto find(string_view_type) const
      -> std::optional<associations_t::const_iterator>;

private:
  associations_t associations;

public:
  auto to_string(const auxilia::FormatPolicy &) const -> string_type;
};
} // namespace accat::lox::evaluation
