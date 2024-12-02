#pragma once
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>

#include "config.hpp"
#include "loxo_fwd.hpp"
#include "utils.hpp"

#include "ScopeEnvironment.hpp"

namespace net::ancillarycat::loxograph {

class Environment : public utils::Printable,
                    public std::enable_shared_from_this<Environment> {
public:
  using eval_result_t = evaluation::ScopeEnvironment::eval_result_t;
  using string_view_type = evaluation::ScopeEnvironment::string_view_type;
  using scope_env_t = evaluation::ScopeEnvironment;
  using scope_env_ptr_t = std::shared_ptr<scope_env_t>;

public:
  Environment();
  explicit Environment(const scope_env_ptr_t &);
  ~Environment() override = default;

public:
  auto find(this auto &&self, const string_type &name)
      -> decltype(self.current->find(name));
  auto add(const string_type &, const eval_result_t &, uint_least32_t)
      -> utils::Status;
  auto reassign(const string_type &, const eval_result_t &, uint_least32_t)
      -> utils::Status;
  auto get(const string_type &) const -> eval_result_t;
  /// @brief used to construct sub-envs from current env
  auto operator*() const -> const scope_env_ptr_t & { return current; }

private:
  scope_env_ptr_t current{nullptr};
  std::weak_ptr<scope_env_t> parent{};

private:
  auto to_string_impl(const utils::FormatPolicy &) const
      -> string_type override;
};
auto Environment::find(this auto &&self, const string_type &name)
    -> decltype(self.current->find(name)) {
  if (auto it = self.current->find(name))
    return it;

  if (auto enclosing = self.parent.lock())
    return enclosing->find(name);

  return {};
}

} // namespace net::ancillarycat::loxograph
