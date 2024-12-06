#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <memory>

#include <net/ancillarycat/utils/Status.hpp>

#include "details/IVisitor.hpp"
#include "details/loxo_fwd.hpp"
#include "Environment.hpp"
#include "Evaluatable.hpp"
namespace net::ancillarycat::loxo {

Environment::Environment() : current(std::make_shared<scope_env_t>()) {}
Environment::Environment(const std::shared_ptr<self_type> &enclosing)
    : current(std::make_shared<scope_env_t>()), parent(enclosing) {}
auto Environment::createGlobalEnvironment()
    -> utils::StatusOr<std::shared_ptr<Environment>> {
  static auto has_init = false;
  if (has_init)
    return utils::InvalidArgument("init global env twice");

  has_init = true;
  auto env = new Environment();
  env->add("clock"s,
           evaluation::Callable::create_native(
               0,
               [](const interpreter &, evaluation::Callable::args_t &) {
                 dbg(trace, "clock() called");
                 return std::chrono::duration_cast<std::chrono::seconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                     .count();
               }))
      .ignore_error();
  env->add("about",
           evaluation::Callable::create_native(
               0,
               [](const interpreter &, evaluation::Callable::args_t &) {
                 return "loxo";
               }))
      .ignore_error();
  return std::make_shared<Environment>(*env);
}
auto Environment::add(const string_type &name,
                      const utils::IVisitor::variant_type &value,
                      const uint_least32_t line) -> utils::Status {
  return current->add(name, value, line);
}
auto Environment::reassign(const string_type &name,
                           const utils::IVisitor::variant_type &value,
                           const uint_least32_t line) const -> utils::Status {
  if (auto it = find(name)) {
    (*it)->second.first = value;
    (*it)->second.second = line;
    return utils::OkStatus();
  }
  return utils::InvalidArgument("variable not defined");
}
auto Environment::get(const string_type &name) const -> utils::IVisitor::variant_type {
  if (auto it = find(name))
    return {(*it)->second.first};

  return {utils::Monostate{}};
}
// NOLINTNEXTLINE
auto Environment::find(const string_type &name) const
    -> std::optional<self_type::scope_env_t::associations_t::iterator> {
  if (auto maybe_it = current->find(name)) {
    // NOLINTNEXTLINE
    dbg_block(if (parent.expired()) return nullptr;
              if (auto another_it = parent.lock()->find(name)) {
                dbg(warn,
                    "variable '{}' is shadowed; previously declared at line {}",
                    name,
                    (*another_it)->second.second);
              })
    return maybe_it;
  }

  if (auto enclosing = parent.lock())
    return enclosing->find(name);

  return std::nullopt;
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
} // namespace net::ancillarycat::loxo
