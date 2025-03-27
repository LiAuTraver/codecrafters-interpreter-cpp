#include <unordered_map>
#include <string>
#include <string_view>
#include <variant>

#include <accat/auxilia/auxilia.hpp>

#include "details/loxo_fwd.hpp"
#include "details/ScopeAssoc.inl"

#include "Evaluatable.hpp"

namespace accat::loxo::evaluation {
auxilia::Status ScopeAssoc::add(const string_type &name,
                              const variant_type &value,
                              const uint_least32_t line) {
  if (associations.contains(name)) {
    (void)0; /// suppress the warning when not in debugging
    /// Scheme allows redefining variables at the top level; so temporarily
    /// we just follow that.
    dbg(warn,
        "The variable {} is already defined in the environment. redefining "
        "it...",
        name)
  }

  associations.insert_or_assign(name.data(), std::pair{value, line});
  return {};
}
auto ScopeAssoc::to_string(const auxilia::FormatPolicy &format_policy) const
    -> string_type {
  return {};
}
} // namespace accat::loxo::evaluation
