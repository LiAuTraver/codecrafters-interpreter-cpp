#include <cstddef>
#include <cstdint>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <memory>

#include <accat/auxilia/auxilia.hpp>

#include "details/IVisitor.hpp"
#include "details/lox_fwd.hpp"
#include "Environment.hpp"
#include "Evaluatable.hpp"

namespace accat::lox {
using auxilia::Status;
using variant_type = IVisitor::variant_type;
using Env = Environment;
using auxilia::FormatPolicy;
using enum auxilia::FormatPolicy;

bool Env::isGlobalScopeInited = false;

Env::Environment(const std::shared_ptr<self_type> &enclosing)
    : parent(enclosing) {}

Env::Environment(Environment &&that) noexcept {
  current = std::move(that.current);
  parent = std::move(that.parent);
}

auto Env::operator=(Env &&that) noexcept -> Env & {
  if (this == &that) {
    return *this;
  }
  this->current = std::move(that.current);
  this->parent = std::move(that.parent);
  return *this;
}
auto Env::initGlobalEnv() -> std::shared_ptr<Env> {
  isGlobalScopeInited = true;
  global_env = std::make_shared<Env>();
  global_env
      ->add("clock"s,
            evaluation::Function::create_native(
                0,
                [](interpreter &,
                   evaluation::Function::args_t &) -> variant_type {
                  dbg(trace, "clock() called")
                  return {evaluation::Number{static_cast<long double>(
                      std::chrono::duration_cast<std::chrono::seconds>(
                          std::chrono::system_clock::now().time_since_epoch())
                          .count())}};
                },
                nullptr))
      .ignore_error();
  global_env
      ->add("about",
            evaluation::Function::create_native(
                0,
                [](interpreter &,
                   evaluation::Function::args_t &) -> variant_type {
                  return {
                      evaluation::String{"lox programming language, based on "
                                         "book Crafting Interpreters."sv}};
                },
                nullptr))
      .ignore_error();
  return global_env;
}
auto Env::Global() -> std::shared_ptr<Env> {
  if (isGlobalScopeInited)
    return global_env;
  return initGlobalEnv();
}

auto Env::Scope(const std::shared_ptr<self_type> &enclosing)
    -> std::shared_ptr<self_type> {
  return std::shared_ptr<Env>(new Env(enclosing));
}

auto Env::add(const string_view_type name,
              const variant_type &value,
              const uint_least32_t line) -> Status {
  return current.add(name, value, line);
}

auto Env::reassign(const string_view_type name,
                   const variant_type &value,
                   const uint_least32_t line,
                   bool currentScopeOnly) -> Status {
  if (const auto it = find(name, currentScopeOnly)) {
    (*it)->second.first = value;
    (*it)->second.second = line;
    return {};
  }
  return auxilia::InvalidArgumentError("variable not defined");
}

auto Env::get(const string_view_type name, const bool currentScopeOnly) const
    -> variant_type* {
  if (auto it = const_cast<Env *>(this)->find(name, currentScopeOnly))
    return &(*it)->second.first;

  return nullptr;
}

auto Env::ancestor(const size_t n) const -> std::shared_ptr<self_type> {
  auto raw_env = const_cast<Env *>(this);
  for (auto _ : std::views::iota(0ull, n)) {
    if (!raw_env or !raw_env->parent)
      return nullptr;
    raw_env = raw_env->parent.get();
  }
  return raw_env->shared_from_this();
}
auto Env::get_at_depth(const size_t n, const string_view_type name) const
    -> variant_type* {
  auto myenv = this->ancestor(n);
  contract_assert(myenv, "ancestor is null")
  const auto it = myenv->find(name, true);
  contract_assert(it, "variable not found")
  return &(*it)->second.first;
}
auto Env::reassign_at_depth(const size_t n,
                            const string_view_type name,
                            const variant_type &value,
                            const uint_least32_t line) -> Status {
  precondition(this->ancestor(n), "ancestor is null")
  return this->ancestor(n)->reassign(name, value, line, true);
}

auto Env::to_string(const FormatPolicy &format_policy) const -> string_type {
  std::ostringstream oss;
  oss << current.to_string(format_policy) << ",\n\t-> ";
  if (const auto enclosing = this->parent.get())
    oss << enclosing->to_string(format_policy);
  else
    oss << "nullptr";
  return oss.str();
}

auto Env::find(const string_view_type name, const bool currentScopeOnly) const
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
auto Env::find(const string_view_type name, const bool currentScopeOnly)
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
auto Environment::find_symbol(const string_view_type name,
                              const bool currentScopeOnly) const
    -> std::optional<self_type::scope_env_t::associations_t::const_iterator> {
  if (auto maybe_it = current.find_symbol(name)) {
    dbg_block
    {
      if (!parent or currentScopeOnly) {
        return;
      }
      if (const auto another_it = parent->find_symbol(name)) {
        dbg(warn,
            "symbol '{}' is shadowed; previously declared at line {}",
            name,
            (*another_it)->second.second);
      }
      return;
    };
    return maybe_it;
  }
  if (!currentScopeOnly)
    if (const auto enclosing = parent.get())
      return enclosing->find_symbol(name, currentScopeOnly);

  return std::nullopt;
}

auto Env::find_symbol(const string_view_type name,
                      const bool currentScopeOnly) -> std::optional<self_type::scope_env_t::associations_t::iterator> {
  if (auto maybe_it = current.find_symbol(name)) {
    dbg_block
    {
      if (!parent or currentScopeOnly) {
        return;
      }
      if (const auto another_it = parent->find_symbol(name)) {
        dbg(warn,
            "symbol '{}' is shadowed; previously declared at line {}",
            name,
            (*another_it)->second.second);
      }
      return;
    };
    return maybe_it;
  }
  if (!currentScopeOnly)
    if (const auto enclosing = parent.get())
      return enclosing->find_symbol(name, currentScopeOnly);

  return std::nullopt;
}
} // namespace accat::lox
