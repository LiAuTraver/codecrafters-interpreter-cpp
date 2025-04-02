#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <memory>

#include <accat/auxilia/auxilia.hpp>

#include "accat/auxilia/details/Status.hpp"
#include "accat/auxilia/details/macros.hpp"
#include "details/IVisitor.hpp"
#include "details/loxo_fwd.hpp"
#include "Environment.hpp"
#include "Evaluatable.hpp"

namespace accat::loxo {

bool Environment::isGlobalScopeInited = false;

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
  if (isGlobalScopeInited)
    return global_env;
  isGlobalScopeInited = true;
  global_env = std::make_shared<Environment>();
  global_env
      ->add(
          "clock"s,
          evaluation::Callable::create_native(
              0,
              [](interpreter &, evaluation::Callable::args_t &) {
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
                [](interpreter &, evaluation::Callable::args_t &) {
                  return IVisitor::variant_type{evaluation::String{
                      "lox programming language, based on "
                      "book Crafting Interpreters's lox."sv}};
                },
                nullptr))
      .ignore_error();
  return global_env;
}

auto Environment::Scope(const std::shared_ptr<self_type> &enclosing)
    -> std::shared_ptr<self_type> {
  return std::shared_ptr<Environment>(new Environment(enclosing));
}

auto Environment::add(const string_view_type name,
                      const IVisitor::variant_type &value,
                      const uint_least32_t line) -> auxilia::Status {
  return current.add(name, value, line);
}

auto Environment::reassign(const string_view_type name,
                           const IVisitor::variant_type &value,
                           const uint_least32_t line,
                           bool currentScopeOnly) -> auxilia::Status {
  if (const auto it = find(name, currentScopeOnly)) {
    (*it)->second.first = value;
    (*it)->second.second = line;
    return {};
  }
  return auxilia::InvalidArgumentError("variable not defined");
}

auto Environment::get(const string_view_type name,
                      const bool currentScopeOnly) const
    -> IVisitor::variant_type {
  if (const auto it = find(name, currentScopeOnly))
    return {(*it)->second.first};

  return {auxilia::Monostate{}};
}

auto Environment::copy() const -> std::shared_ptr<self_type> {
  // return std::make_shared<self_type>(*this);
  TODO("^^^ failed to compile")
}
auto Environment::ancestor(const size_t n) const -> std::shared_ptr<self_type> {
  auto raw_env = const_cast<Environment *>(this);
  for (auto _ : std::views::iota(0ull, n)) {
    if (!raw_env or !raw_env->parent)
      return nullptr;
    raw_env = raw_env->parent.get();
  }
  return raw_env->shared_from_this();
}
auto Environment::get_at_depth(const size_t n,
                               const string_view_type name) const
    -> IVisitor::variant_type {
  auto myenv = this->ancestor(n);
  contract_assert(myenv, "ancestor is null")
  const auto it = myenv->find(name, true);
  contract_assert(it, "variable not found")
  return (*it)->second.first;
}
auto Environment::reassign_at_depth(const size_t n,
                                    const string_view_type name,
                                    const IVisitor::variant_type &value,
                                    const uint_least32_t line)
    -> auxilia::Status {
  precondition(this->ancestor(n), "ancestor is null")
  return this->ancestor(n)->reassign(name, value, line, true);
}

auto Environment::to_string(const auxilia::FormatPolicy &format_policy) const
    -> string_type {
  string_type result;
  result += current.to_string(auxilia::FormatPolicy::kDetailed);
  if (const auto enclosing = this->parent.get()) {
    result += enclosing->to_string(auxilia::FormatPolicy::kDetailed);
  }
  return result;
}

auto Environment::find(const string_view_type name,
                       const bool currentScopeOnly) const
    -> std::optional<self_type::scope_env_t::associations_t::const_iterator> {
  if (auto maybe_it = current.find(name)) {
    dbg_block
    {
      if (!parent or currentScopeOnly) {
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
  if (!currentScopeOnly)
    if (const auto enclosing = parent.get())
      return enclosing->find(name, currentScopeOnly);

  return std::nullopt;
}
auto Environment::find(const string_view_type name, const bool currentScopeOnly)
    -> std::optional<self_type::scope_env_t::associations_t::iterator> {
  if (auto maybe_it = current.find(name)) {
    dbg_block
    {
      if (!parent or currentScopeOnly) {
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
  if (!currentScopeOnly)
    if (const auto enclosing = parent.get())
      return enclosing->find(name, currentScopeOnly);

  return std::nullopt;
}
} // namespace accat::loxo
