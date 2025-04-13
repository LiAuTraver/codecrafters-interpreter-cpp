#ifndef AC_LOX_ENVIRONMENT_HPP
#define AC_LOX_ENVIRONMENT_HPP

#include <cstddef>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>

#include <accat/auxilia/auxilia.hpp>

#include "details/lox_fwd.hpp"

#include "details/IVisitor.hpp"
#include "details/ScopeAssoc.inl"

namespace accat::lox {

class AC_LOX_API Environment
    : public auxilia::Printable,
      public std::enable_shared_from_this<Environment> {
public:
  using string_view_type = evaluation::ScopeAssoc::string_view_type;
  using scope_env_t = evaluation::ScopeAssoc;
  using self_type = Environment;

public:
  Environment() = default;
  Environment(const Environment &) = delete;
  auto operator=(const Environment &) = delete;
  Environment(Environment &&) noexcept;
  auto operator=(Environment &&) noexcept -> Environment &;
  ~Environment() = default;

private:
  explicit Environment(const std::shared_ptr<self_type> &);

public:
  static auto Global() -> std::shared_ptr<self_type>;
  static auto Scope(const std::shared_ptr<self_type> &)
      -> std::shared_ptr<self_type>;
  static bool isGlobalScopeInited;

public:
  auto find(string_view_type, bool = false) const
      -> std::optional<self_type::scope_env_t::associations_t::const_iterator>;
  auto find(string_view_type, bool = false)
      -> std::optional<self_type::scope_env_t::associations_t::iterator>;
  auto add(string_view_type,
           const IVisitor::variant_type &,
           uint_least32_t = std::numeric_limits<uint_least32_t>::quiet_NaN())
      -> auxilia::Status;
  auto reassign(string_view_type,
                const IVisitor::variant_type &,
                uint_least32_t,
                bool = false) -> auxilia::Status;
  auto get(string_view_type, bool = false) const -> IVisitor::variant_type *;
  auto ancestor(size_t) const -> std::shared_ptr<self_type>;
  auto get_at_depth(size_t, string_view_type) const -> IVisitor::variant_type *;
  auto reassign_at_depth(size_t,
                         string_view_type,
                         const IVisitor::variant_type &,
                         uint_least32_t) -> auxilia::Status;

private:
  scope_env_t current;
  std::shared_ptr<self_type> parent;
  static inline std::shared_ptr<self_type> global_env;
  static auto initGlobalEnv() -> std::shared_ptr<self_type>;

public:
  /// @note: use `env._Ptr->to_string(accat::auxilia::FormatPolicy::kDefault)` in debug
  dbg_only([[gnu::used]])
  auto to_string(const auxilia::FormatPolicy &) const -> string_type;
};
} // namespace accat::lox

#endif // AC_LOX_ENVIRONMENT_HPP
