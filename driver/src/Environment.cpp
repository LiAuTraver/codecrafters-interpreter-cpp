#include <unordered_map>
#include <string>
#include <string_view>
#include <variant>

#include "config.hpp"
#include "loxo_fwd.hpp"
#include "fmt.hpp"
#include "Environment.hpp"

#include "Variant.hpp"

namespace net::ancillarycat::loxograph::evaluation {
utils::Status Environment::add(const string_type &name,
                               const eval_result_t &value) {
  if (associations.contains(name)) {
    /// Scheme allows redefining variables at the top level; so temporarily we
    /// just follow that.
    dbg(warn,
        "The variable {} is already defined in the environment. redefining "
        "it...",
        name);
  }
  associations.insert_or_assign(name.data(), value);
  return utils::OkStatus();
}
Environment::eval_result_t Environment::get(const string_type &name) const {
  if (auto it = associations.find({name.begin(),name.end()}); it != associations.end()) {
    dbg(trace, "Found the variable {} in the environment.", name);
    return it->second;
  }
  return {Error{utils::format(
      "The variable `{}` is not defined in the environment.", name)}};
}
auto Environment::to_string_impl(const utils::FormatPolicy &format_policy) const
    -> string_type {
  return {};
}
} // namespace net::ancillarycat::loxograph::evaluation
