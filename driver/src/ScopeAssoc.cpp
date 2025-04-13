#include <sstream>
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
  if (is_symbol(value))
    return add_symbol(name, value, line);

  if (variables.contains({name.data(), name.size()})) {
    /// Scheme allows redefining variables at the top level; so temporarily
    /// we just follow that.
    dbg(warn,
        "The variable {} is already defined in the environment. redefining "
        "it...",
        name)
  }
  variables.insert_or_assign(name.data(), std::pair{value, line});
  return {};
}
auto ScopeAssoc::add_symbol(string_view_type name,
                            const variant_type &value,
                            uint_least32_t line) -> auxilia::Status {
  if (auto it = symbols.find({name.data(), name.size()}); it != symbols.end()) {
    if (it->second.first.index() != value.index()) {
      return auxilia::InvalidArgumentError(
          "redefine a symbol with a different type is not "
          "allowed");
    }
    dbg(warn,
        "The symbol {} is already defined in the environment. redefining "
        "it...",
        name)
  }
  symbols.insert_or_assign(name.data(), std::pair{value, line});
  return {};
}
auto ScopeAssoc::to_string(const auxilia::FormatPolicy &format_policy) const
    -> string_type {
  std::ostringstream oss;
  oss << "[ ";
  for (const auto &[key, value] : variables) {
    oss << key << ": " << value.first.to_string(format_policy) << ", ";
  }
  for (const auto &[key, value] : symbols) {
    oss << key << ": " << value.first.to_string(format_policy) << ", ";
  }

  if (variables.size() + symbols.size() == 0)
    oss.seekp(-2, std::ios_base::end);
  else
    oss << " ]";

  oss << auxilia::format(" ({} entries) ", variables.size());
  return oss.str();
}
auto ScopeAssoc::find(const string_view_type name)
    -> std::optional<associations_t::iterator> {
  if (auto it = variables.find({name.data(), name.size()});
      it != variables.end())
    return {it};
  if (auto it = symbols.find({name.data(), name.size()}); it != symbols.end())
    return {it};
  return std::nullopt;
}
auto ScopeAssoc::find(const string_view_type name) const
    -> std::optional<associations_t::const_iterator> {
  if (auto it = variables.find({name.data(), name.size()});
      it != variables.end())
    return {it};
  if (auto it = symbols.find({name.data(), name.size()}); it != symbols.end())
    return {it};
  return std::nullopt;
}
auto ScopeAssoc::find_symbol(const string_view_type name)
    -> std::optional<associations_t::iterator> {
  if (auto it = symbols.find({name.data(), name.size()}); it != symbols.end())
    return {it};
  return std::nullopt;
}
auto ScopeAssoc::find_symbol(const string_view_type name) const
    -> std::optional<associations_t::const_iterator> {
  if (auto it = symbols.find({name.data(), name.size()}); it != symbols.end())
    return {it};
  return std::nullopt;
}
bool ScopeAssoc::is_symbol(const variant_type &value) const {
  return value.is_type<Class>() or value.is_type<Function>();
}
} // namespace accat::lox::evaluation
