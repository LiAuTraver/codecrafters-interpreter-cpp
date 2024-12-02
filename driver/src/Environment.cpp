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
                               const eval_result_t &value,
                               const uint_least32_t line) {
  if (associations.contains(name))
    (void)0, /// suppress the warning when not in debugging
        /// Scheme allows redefining variables at the top level; so temporarily
        /// we just follow that.
        dbg(warn,
            "The variable {} is already defined in the environment. redefining "
            "it...",
            name);

  associations.insert_or_assign(name.data(), std::pair{value, line});
  return utils::OkStatus();
}
utils::Status Environment::reassign(const string_type &name,
                                    const eval_result_t &value,
                                    const uint_least32_t line) {
  if (auto it = this->find(name)) {
    (*it)->second.first = value;
    (*it)->second.second = line;
    dbg(info, "Changed the value of the variable {} in the environment.", name);
    return utils::OkStatus();
  }
  dbg(error, "The variable {} is not defined in the environment.", name);
  return utils::InvalidArgument("variable not defined");
}
Environment::eval_result_t Environment::get(const string_type &name) const {
  if (auto it = this->find(name)) {
    dbg(trace, "Found the variable {} in the environment.", name);
    return (*it)->second.first;
  }
  dbg(error, "The variable {} is not defined in the environment.", name);
  return {utils::Monostate{}};
}
auto Environment::find(this auto &&self, const string_type &name)
    -> std::optional<decltype(self.associations.find(name))> {
  auto it = self.associations.find(name);
  if (it == self.associations.end())
    return std::nullopt;
  return it;
}
auto Environment::to_string_impl(const utils::FormatPolicy &format_policy) const
    -> string_type {
  return {};
}
} // namespace net::ancillarycat::loxograph::evaluation
