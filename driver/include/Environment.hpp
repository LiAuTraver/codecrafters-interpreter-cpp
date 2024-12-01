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
public:
  constexpr Environment() = default;
  virtual ~Environment() override = default;
public:
  utils::Status add(const string_type &name, const eval_result_t &value, uint_least32_t line);
  eval_result_t get(const string_type &name) const;

private:
  std::unordered_map<string_type, std::pair<eval_result_t,uint_least32_t>> associations{};
private:
  auto to_string_impl(const utils::FormatPolicy &format_policy) const
      -> string_type override;
};

} // namespace net::ancillarycat::loxograph::evaluation
