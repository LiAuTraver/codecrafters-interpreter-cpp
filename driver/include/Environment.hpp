#pragma once
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>

#include "config.hpp"
#include "loxo_fwd.hpp"
#include "status.hpp"
#include "utils.hpp"

#include "ScopeAssoc.hpp"

namespace net::ancillarycat::loxo {

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
  virtual ~Environment() override = default;

public:
  static auto createGlobalEnvironment()
      -> utils::StatusOr<std::shared_ptr<self_type>>;

public:
  auto find(const string_type &name) const
      -> std::optional<self_type::scope_env_t::associations_t::iterator>;
  auto add(const string_type &,
           const eval_result_t &,
           uint_least32_t = std::numeric_limits<uint_least32_t>::quiet_NaN())
      -> utils::Status;
  auto reassign(const string_type &, const eval_result_t &, uint_least32_t) const
      -> utils::Status;
  auto get(const string_type &) const -> eval_result_t;

private:
  scope_env_ptr_t current{};
  std::weak_ptr<self_type> parent{};

private:
  auto to_string_impl(const utils::FormatPolicy &) const
      -> string_type override;
};
} // namespace net::ancillarycat::loxo
