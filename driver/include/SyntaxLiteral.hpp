#pragma once

#include <random>

#include "config.hpp"
#include "loxo_fwd.hpp"

/// @namespace net::ancillarycat::loxograph::syntax
/// @note this is a `rescue` method beacuse original token solution was a mess;
///       you can't evaluate the result in `evaluate` method; yet it went well
///       when just parsing and lexing.
namespace net::ancillarycat::loxograph::syntax {
class SyntaxLiteral;
class Keyword;
class String;
class Number;
class Value;
class False;
class SyntaxLiteral : public utils::Printable {
public:
  constexpr SyntaxLiteral() = default;
  virtual ~SyntaxLiteral() override = default;
};
class String : public SyntaxLiteral, public utils::Viewable {
public:
  constexpr String() = default;
  String(const string_type &value) : value(value) {}
  String(const string_view_type value) : value(value) {}
  virtual ~String() override = default;

public:
  String &operator+(const String &rhs) {
    value += rhs.value;
    return *this;
  }

private:
  auto to_string_impl(const utils::FormatPolicy &format_policy) const
      -> string_type override {
    return value;
  }
  auto to_string_view_impl(const utils::FormatPolicy &format_policy) const
      -> string_view_type override {
    return value;
  }

private:
  string_type value;
};
class Keyword : public SyntaxLiteral, public utils::Viewable {
public:
  constexpr Keyword() = default;
  virtual ~Keyword() override = default;
};
class Value : public SyntaxLiteral, public utils::Viewable {
public:
  constexpr Value() = default;
  virtual ~Value() override = default;
};
class Nil : public Value {
public:
  constexpr Nil() = default;
  virtual ~Nil() = default;

private:
  auto to_string_impl(const utils::FormatPolicy &format_policy) const
      -> string_type override {
    return "nil"s;
  }
  auto to_string_view_impl(const utils::FormatPolicy &format_policy) const
      -> string_view_type override {
    return "nil"sv;
  }
};
class Boolean : public Value {
public:
  constexpr Boolean() = default;
  constexpr Boolean(const bool value) : value(value) {}
  constexpr Boolean(const long double value) : value(!!value) {}
  Boolean(const Boolean &value) : value(value.value) {}
  Boolean &operator=(const Boolean &value) {
    this->value = value.value;
    return *this;
  }
  Boolean(Boolean &&value) noexcept : value(std::move(value.value)) {}
  Boolean &operator=(Boolean &&value) noexcept {
    this->value = std::move(value.value);
    return *this;
  }
  auto operator==(const Boolean &rhs) const -> bool {
    return value == rhs.value;
  }
  auto operator<=>(const Boolean &rhs) const -> std::strong_ordering {
    return value <=> rhs.value;
  }
  auto operator!() const -> Boolean { return Boolean{!value}; }

  virtual ~Boolean() = default;

private:
  auto to_string_impl(const utils::FormatPolicy &format_policy) const
      -> string_type override {
      contract_assert(value.has_value());
    return value.value() ? "true"s : "false"s;
  }
  auto to_string_view_impl(const utils::FormatPolicy &format_policy) const -> string_view_type override{
    contract_assert(value.has_value());
    return value.value() ? "true"sv : "false"sv;
  }

private:
  std::optional<bool> value = std::nullopt;
};
static constexpr auto True = Boolean{true};
static constexpr auto False = Boolean{false};
inline constexpr auto operator!(const Nil &) noexcept -> Boolean { return True; }
class Number : public Value {
public:
  constexpr Number() = default;
  Number(const long double value) : value(value) {}
  virtual ~Number() override = default;

private:
  long double value = std::numeric_limits<long double>::quiet_NaN();

private:
  auto to_string_impl(const utils::FormatPolicy &format_policy) const
      -> string_type override {
    auto buffer = string_type{32, '\0'};
    if (auto [ptr, ec] =
            std::to_chars(buffer.data(), buffer.data() + buffer.size(), value);
        ec == std::errc{}) {
      // to_chars can format `NaN` and signaling `NaN` as `nan` and `nan(s)`
      // respectively ^^^
      buffer.resize(ptr - buffer.data());
      dbg(info, "Num: ", buffer);
    } else {
      dbg_block(dbg(critical,
                    "this shall be unreachable. if a number failed to convert, "
                    "it shall "
                    "be handled in the previous stage.");
                contract_assert(false););
      buffer.clear();
      buffer = "<failed to convert number to string>";
    }
    return buffer;
  }
};
class And : public Keyword {
public:
  constexpr And() = default;
  virtual ~And() = default;

private:
  auto to_string_impl(const utils::FormatPolicy &format_policy) const
      -> string_type override {
    return "and"s;
  }
  auto to_string_view_impl(const utils::FormatPolicy &format_policy) const
      -> string_view_type override {
    return "and"sv;
  }
};
class Or : public Keyword {
public:
  constexpr Or() = default;
  virtual ~Or() = default;

private:
  auto to_string_impl(const utils::FormatPolicy &format_policy) const
      -> string_type override {
    return "or"s;
  }
  auto to_string_view_impl(const utils::FormatPolicy &format_policy) const
      -> string_view_type override {
    return "or"sv;
  }
};
class Class : public Keyword {
public:
  constexpr Class() = default;
  virtual ~Class() = default;

private:
  auto to_string_impl(const utils::FormatPolicy &format_policy) const
      -> string_type override {
    return "class"s;
  }
  auto to_string_view_impl(const utils::FormatPolicy &format_policy) const
      -> string_view_type override {
    return "class"sv;
  }
};

class Else : public Keyword {
public:
  constexpr Else() = default;
  virtual ~Else() = default;

private:
  auto to_string_impl(const utils::FormatPolicy &format_policy) const
      -> string_type override {
    return "else"s;
  }
  auto to_string_view_impl(const utils::FormatPolicy &format_policy) const
      -> string_view_type override {
    return "else"sv;
  }
};

class If : public Keyword {
public:
  constexpr If() = default;
  virtual ~If() = default;

private:
  auto to_string_impl(const utils::FormatPolicy &format_policy) const
      -> string_type override {
    return "if"s;
  }
  auto to_string_view_impl(const utils::FormatPolicy &format_policy) const
      -> string_view_type override {
    return "if"sv;
  }
};

class For : public Keyword {
public:
  constexpr For() = default;
  virtual ~For() = default;

private:
  auto to_string_impl(const utils::FormatPolicy &format_policy) const
      -> string_type override {
    return "for"s;
  }
  auto to_string_view_impl(const utils::FormatPolicy &format_policy) const
      -> string_view_type override {
    return "for"sv;
  }
};

class While : public Keyword {
public:
  constexpr While() = default;
  virtual ~While() = default;

private:
  auto to_string_impl(const utils::FormatPolicy &format_policy) const
      -> string_type override {
    return "while"s;
  }
  auto to_string_view_impl(const utils::FormatPolicy &format_policy) const
      -> string_view_type override {
    return "while"sv;
  }
};

class Return : public Keyword {
public:
  constexpr Return() = default;
  virtual ~Return() = default;

private:
  auto to_string_impl(const utils::FormatPolicy &format_policy) const
      -> string_type override {
    return "return"s;
  }
  auto to_string_view_impl(const utils::FormatPolicy &format_policy) const
      -> string_view_type override {
    return "return"sv;
  }
};

class Fun : public Keyword {
public:
  constexpr Fun() = default;
  virtual ~Fun() = default;

private:
  auto to_string_impl(const utils::FormatPolicy &format_policy) const
      -> string_type override {
    return "fun"s;
  }
  auto to_string_view_impl(const utils::FormatPolicy &format_policy) const
      -> string_view_type override {
    return "fun"sv;
  }
};

class Var : public Keyword {
public:
  constexpr Var() = default;
  virtual ~Var() = default;

private:
  auto to_string_impl(const utils::FormatPolicy &format_policy) const
      -> string_type override {
    return "var"s;
  }
  auto to_string_view_impl(const utils::FormatPolicy &format_policy) const
      -> string_view_type override {
    return "var"sv;
  }
};

class This : public Keyword {
public:
  constexpr This() = default;
  virtual ~This() = default;

private:
  auto to_string_impl(const utils::FormatPolicy &format_policy) const
      -> string_type override {
    return "this"s;
  }
  auto to_string_view_impl(const utils::FormatPolicy &format_policy) const
      -> string_view_type override {
    return "this"sv;
  }
};

class Super : public Keyword {

public:
  constexpr Super() = default;
  virtual ~Super() = default;

private:
  auto to_string_impl(const utils::FormatPolicy &format_policy) const
      -> string_type override {
    return "super"s;
  }
  auto to_string_view_impl(const utils::FormatPolicy &format_policy) const
      -> string_view_type override {
    return "super"sv;
  }
};

class Print : public Keyword {
public:
  constexpr Print() = default;
  virtual ~Print() = default;

private:
  auto to_string_impl(const utils::FormatPolicy &format_policy) const
      -> string_type override {
    return "print"s;
  }
  auto to_string_view_impl(const utils::FormatPolicy &format_policy) const
      -> string_view_type override {
    return "print"sv;
  }
};
} // namespace net::ancillarycat::loxograph::syntax
