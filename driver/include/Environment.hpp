#pragma once
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>

#include "config.hpp"
#include "interpreter.hpp"
#include "loxo_fwd.hpp"
#include "status.hpp"
#include "utils.hpp"

#include "ScopeAssoc.hpp"

namespace net::ancillarycat::loxograph {

class Environment : public utils::Printable,
                    public std::enable_shared_from_this<Environment> {
public:
  using eval_result_t = evaluation::ScopeAssoc::eval_result_t;
  using string_view_type = evaluation::ScopeAssoc::string_view_type;
  using scope_env_t = evaluation::ScopeAssoc;
  using scope_env_ptr_t = std::shared_ptr<scope_env_t>;
  using self_type = Environment;

public:
  Environment();
  explicit Environment(const std::shared_ptr<self_type> &);
  ~Environment() override = default;

public:
  static auto createGlobalEnvironment()
      -> utils::StatusOr<std::shared_ptr<Environment>>;

public:
  auto find(this auto &&self, const string_type &name)
      -> decltype(self.current->find(name));
  auto add(const string_type &,
           const eval_result_t &,
           uint_least32_t = std::numeric_limits<uint_least32_t>::quiet_NaN())
      -> utils::Status;
  auto reassign(const string_type &, const eval_result_t &, uint_least32_t)
      -> utils::Status;
  auto get(const string_type &) const -> eval_result_t;

private:
  scope_env_ptr_t current{};
  std::weak_ptr<self_type> parent{};

private:
  auto to_string_impl(const utils::FormatPolicy &) const
      -> string_type override;
};
auto Environment::find(this auto &&self, const string_type &name)
    -> decltype(self.current->find(name)) {
  if (auto it = self.current->find(name)) {
    dbg_block(if (self.parent.expired()) return nullptr;
              if (auto another_it = self.parent.lock()->find(name)) {
                dbg(warn,
                    "variable '{}' is shadowed; previously declared at line {}",
                    name,
                    (*another_it)->second.second);
              })
    return {it};
  }

  if (auto enclosing = self.parent.lock())
    return enclosing->find(name);

  return std::nullopt;
}

} // namespace net::ancillarycat::loxograph
