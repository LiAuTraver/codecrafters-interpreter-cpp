#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

#include "config.hpp"
#include "loxo_fwd.hpp"
#include "utils.hpp"
#include "Variant.hpp"

#include "Environment.hpp"

#include <memory>

namespace net::ancillarycat::loxograph {

Environment::Environment() : current(std::make_shared<scope_env_t>()) {}
Environment::Environment(const std::shared_ptr<self_type> &enclosing)
    : current(std::make_shared<scope_env_t>()), parent(enclosing) {}
auto Environment::createGlobalEnvironment() -> utils::StatusOr<std::shared_ptr<Environment>> {
  static auto has_init = false;
  if (has_init) {
    return utils::InvalidArgument("init global env twice");
  }
  has_init = true;
  auto env = new Environment();
  env->add("clock"s,
          evaluation::Callable::create_native(
              [](const interpreter &, evaluation::Callable::args_t &) {
                return std::chrono::duration_cast<std::chrono::milliseconds>(
                           std::chrono::system_clock::now().time_since_epoch())
                    .count();
              }))
      .ignore_error();
  return std::make_shared<Environment>(*env);
}
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
