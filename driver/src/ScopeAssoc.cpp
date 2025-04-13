#include <unordered_map>
#include <string>
#include <string_view>
#include <variant>

#include <accat/auxilia/auxilia.hpp>

#include "details/lox_fwd.hpp"
#include "details/ScopeAssoc.inl"

#include "Evaluatable.hpp"

namespace accat::lox::evaluation {
auxilia::Status ScopeAssoc::add(string_view_type name,
                              const variant_type &value,
                              const uint_least32_t line) {
  if (associations.contains({name.data(), name.size()})) {
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
  string_type result;
  result += "[ ";
  for (const auto &[key, value] : associations) {
    result += key;
    result += ": ";
    result += value.first.to_string(format_policy);
    result += ", ";
  }
  if (!associations.empty()){
    result.pop_back();
    result.pop_back();
  }
  result += auxilia::format(" ] ({} entries) ", associations.size());
  
  return result;
}
auto ScopeAssoc::find(const string_view_type name)
    -> std::optional<associations_t::iterator> {
  if (auto it = associations.find({name.data(), name.size()});
      it != associations.end())
    return {it};
  return std::nullopt;
}
auto ScopeAssoc::find(const string_view_type name) const
    -> std::optional<associations_t::const_iterator> {
  if (auto it = associations.find({name.data(), name.size()});
      it != associations.end())
    return {it};
  return std::nullopt;
}
} // namespace accat::lox::evaluation
