#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <memory>

#include <accat/auxilia/auxilia.hpp>

#include "accat/auxilia/details/Status.hpp"
#include "details/IVisitor.hpp"
#include "details/loxo_fwd.hpp"
#include "Environment.hpp"
#include "Evaluatable.hpp"

namespace accat::loxo {

Environment::Environment(const std::shared_ptr<self_type> &enclosing)
    : parent(enclosing) {}

Environment::Environment(Environment &&that) noexcept {
  current = std::move(that.current);
  parent = std::move(that.parent);
}

auto Environment::operator=(Environment &&that) noexcept -> Environment & {
  if (this == &that) {
    return *this;
  }
  this->current = std::move(that.current);
  this->parent = std::move(that.parent);
  return *this;
}

auto Environment::Global() -> std::shared_ptr<Environment> {
  static auto has_init = false;
  if (has_init)
    return global_env;
  has_init = true;
  global_env = std::make_shared<Environment>();
  global_env
      ->add(
          "clock"s,
          evaluation::Callable::create_native(
              0,
              [](const interpreter &, evaluation::Callable::args_t &) {
                dbg(trace, "clock() called")
                return IVisitor::variant_type{
                    evaluation::Number{static_cast<long double>(
                        std::chrono::duration_cast<std::chrono::seconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count())}};
              },
              nullptr))
      .ignore_error();
  global_env
      ->add("about",
            evaluation::Callable::create_native(
                0,
                [](const interpreter &, evaluation::Callable::args_t &) {
                  return IVisitor::variant_type{evaluation::String{
                      "loxo programming language, based on "
                      "book Crafting Interpreters's lox."sv}};
                },
                nullptr))
      .ignore_error();
  return global_env;
}

auto Environment::createScopeEnvironment(
    const std::shared_ptr<self_type> &enclosing) -> std::shared_ptr<self_type> {
  TODO()
}

auto Environment::add(const string_type &name,
                      const IVisitor::variant_type &value,
                      const uint_least32_t line) const -> auxilia::Status {
  return current.add(name, value, line);
}

auto Environment::reassign(const string_type &name,
                           const IVisitor::variant_type &value,
                           const uint_least32_t line) const -> auxilia::Status {
  if (const auto it = find(name)) {
    (*it)->second.first = value;
    (*it)->second.second = line;
    return {};
  }
  return auxilia::InvalidArgumentError("variable not defined");
}

auto Environment::get(const string_type &name) const -> IVisitor::variant_type {
  if (const auto it = find(name))
    return {(*it)->second.first};

  return {auxilia::Monostate{}};
}

// NOLINTNEXTLINE
auto Environment::find(const string_type &name) const
    -> std::optional<self_type::scope_env_t::associations_t::iterator> {
  if (auto maybe_it = current.find(name)) {
    // NOLINTNEXTLINE
    dbg_block
    {
      if (!parent) {
        return;
      }
      if (const auto another_it = parent->find(name)) {
        dbg(warn,
            "variable '{}' is shadowed; previously declared at line {}",
            name,
            (*another_it)->second.second);
      }
      return;
    };
    return maybe_it;
  }

  if (const auto enclosing = parent.get()) {
    return enclosing->find(name);
  }

  return std::nullopt;
}

auto Environment::copy() const -> std::shared_ptr<self_type> {
  // return std::make_shared<self_type>(*this);
  TODO("^^^ failed to compile")
}
// NOLINTNEXTLINE
auto Environment::to_string(const auxilia::FormatPolicy &format_policy) const
    -> string_type {
  string_type result;
  result += current.to_string(auxilia::FormatPolicy::kDetailed);
  if (const auto enclosing = this->parent.get()) {
    result += enclosing->to_string(auxilia::FormatPolicy::kDetailed);
  }
  return result;
}
} // namespace accat::loxo
