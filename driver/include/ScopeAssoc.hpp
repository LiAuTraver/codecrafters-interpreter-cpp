#pragma once
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <unordered_map>
#include <string>
#include <string_view>
#include <variant>

#include "loxo_fwd.hpp"

#include <net/ancillarycat/utils/Status.hpp>
#include "Evaluatable.hpp"

namespace net::ancillarycat::loxo::evaluation {
class ScopeAssoc : utils::Printable {
  friend class ::net::ancillarycat::loxo::Environment;

public:
  using eval_result_t = utils::VisitorBase::eval_result_t;
  using string_view_type = utils::VisitorBase::string_view_type;
  using association_t =
      std::pair<string_type, std::pair<eval_result_t, uint_least32_t>>;
  using associations_t =
      std::unordered_map<string_type, std::pair<eval_result_t, uint_least32_t>>;

public:
  constexpr ScopeAssoc() = default;
  virtual ~ScopeAssoc() override = default;

private:
  auto add(const string_type &,
           const eval_result_t &,
           uint_least32_t = std::numeric_limits<uint_least32_t>::quiet_NaN())
      -> utils::Status;
  auto find(this auto &&self, const string_type &name)
      -> std::optional<associations_t::iterator>;

private:
  associations_t associations{};

private:
  auto to_string_impl(const utils::FormatPolicy &) const
      -> string_type override;
};
auto ScopeAssoc::find(this auto &&self, const string_type &name)
    -> std::optional<associations_t::iterator> {
  if (auto it = self.associations.find(name); it != self.associations.end())
    return it;
  return std::nullopt;
}
} // namespace net::ancillarycat::loxo::evaluation
