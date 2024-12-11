#ifndef AC_LOXO_ENVIRONMENT_HPP
#define AC_LOXO_ENVIRONMENT_HPP

#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>

#include <net/ancillarycat/utils/Status.hpp>

#include "details/loxo_fwd.hpp"

#include "details/IVisitor.hpp"
#include "details/ScopeAssoc.inl"

namespace net::ancillarycat::loxo {

class Environment : public utils::Printable,
                    public std::enable_shared_from_this<Environment> {
public:
  using string_view_type = evaluation::ScopeAssoc::string_view_type;
  using scope_env_t = evaluation::ScopeAssoc;
  using self_type = Environment;

public:
  Environment() = default;
  explicit Environment(const std::shared_ptr<self_type> &);
  Environment(const Environment &) = delete;
  auto operator=(const Environment &) = delete;
  Environment(Environment &&) noexcept;
  auto operator=(Environment &&) noexcept -> Environment &;
  virtual ~Environment() override = default;

public:
  static auto getGlobalEnvironment()
      -> utils::StatusOr<std::shared_ptr<self_type>>;
  static auto createScopeEnvironment(const std::shared_ptr<self_type> &)
      -> std::shared_ptr<self_type>;

public:
  auto find(const string_type &) const
      -> std::optional<self_type::scope_env_t::associations_t::iterator>;
  auto
  add(const string_type &,
      const utils::IVisitor::variant_type &,
      uint_least32_t = std::numeric_limits<uint_least32_t>::quiet_NaN()) const
      -> utils::Status;
  auto reassign(const string_type &,
                const utils::IVisitor::variant_type &,
                uint_least32_t) const -> utils::Status;
  auto get(const string_type &) const -> utils::IVisitor::variant_type;
  auto copy() const -> std::shared_ptr<self_type>;

private:
  mutable scope_env_t current;
  std::shared_ptr<self_type> parent;
  static inline std::shared_ptr<self_type> global_env;

private:
  auto to_string_impl(const utils::FormatPolicy &) const
      -> string_type override;
};
} // namespace net::ancillarycat::loxo

#endif // AC_LOXO_ENVIRONMENT_HPP
