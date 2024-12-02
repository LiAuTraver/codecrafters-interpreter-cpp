#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

#include "config.hpp"
#include "loxo_fwd.hpp"
#include "utils.hpp"
#include "Variant.hpp"

#include "Environment.hpp"

namespace net::ancillarycat::loxograph {

auto Environment::add(const string_type &name,
                      const eval_result_t &value,
                      const uint_least32_t line) -> utils::Status {
  return current->add(name, value, line);
}
auto Environment::reassign(const string_type &name,
                           const eval_result_t &value,
                           const uint_least32_t line) -> utils::Status {
  if (auto it = find(name)) {
    (*it)->second.first = value;
    (*it)->second.second = line;
    return utils::OkStatus();
  }
  return utils::InvalidArgument("variable not defined");
}
auto Environment::get(const string_type &name) const -> eval_result_t {
  if (auto it = find(name))
    return {(*it)->second.first};

  return {utils::Monostate{}};
}
auto Environment::to_string_impl(const utils::FormatPolicy &) const
    -> string_type {
  string_type result;
  result += current->to_string(utils::FormatPolicy::kTokenOnly);
  if (auto enclosing = this->parent.lock()) {
    result += enclosing->to_string(utils::FormatPolicy::kTokenOnly);
  }
  return result;
}
} // namespace net::ancillarycat::loxograph
