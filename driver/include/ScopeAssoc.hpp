#pragma once
#include <cstddef>
#include <functional>
#include <optional>
#include <unordered_map>
#include <string>
#include <string_view>
#include <variant>

#include "config.hpp"
#include "loxo_fwd.hpp"
#include "utils.hpp"
#include "status.hpp"
#include "Evaluatable.hpp"

namespace net::ancillarycat::loxograph::evaluation {
class ScopeAssoc : utils::Printable {
  friend class ::net::ancillarycat::loxograph::Environment;

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
  auto add(const string_type &, const eval_result_t &, uint_least32_t)
      -> utils::Status;
  auto find(this auto &&self, const string_type &name)
      -> std::optional<decltype(self.associations.find(name))>;

private:
  associations_t associations{};

private:
  auto to_string_impl(const utils::FormatPolicy &) const
      -> string_type override;
};
auto ScopeAssoc::find(this auto &&self, const string_type &name)
    -> std::optional<decltype(self.associations.find(name))> {
  if (auto it = self.associations.find(name); it != self.associations.end())
    return it;
  return std::nullopt;
}
} // namespace net::ancillarycat::loxograph::evaluation
