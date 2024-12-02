#pragma once
#include <unordered_map>
#include <string>
#include <string_view>
#include <variant>

#include "config.hpp"
#include "loxo_fwd.hpp"
#include "fmt.hpp"
#include "status.hpp"
#include "Evaluatable.hpp"

namespace net::ancillarycat::loxograph::evaluation {
class Environment : utils::Printable {
public:
  using eval_result_t = utils::VisitorBase::eval_result_t;
  using string_view_type = utils::VisitorBase::string_view_type;
  using association_t =
      std::pair<string_type, std::pair<eval_result_t, uint_least32_t>>;
  using associations_t =
      std::unordered_map<string_type, std::pair<eval_result_t, uint_least32_t>>;

public:
  constexpr Environment() = default;
  virtual ~Environment() override = default;

public:
  auto add(const string_type &, const eval_result_t &, uint_least32_t)
      -> utils::Status;
  auto reassign(const string_type &, const eval_result_t &, uint_least32_t)
      -> utils::Status;
  auto get(const string_type &) const -> eval_result_t;

private:
  associations_t associations{};

private:
  auto find(this auto &&self, const string_type &name)
      -> std::optional<decltype(self.associations.find(name))>;

private:
  auto to_string_impl(const utils::FormatPolicy &) const
      -> string_type override;
};

} // namespace net::ancillarycat::loxograph::evaluation
