#pragma once

#include <random>

#include "config.hpp"
#include "loxo_fwd.hpp"

/// @namespace net::ancillarycat::loxograph::syntax
/// @note this is a `rescue` method beacuse original token solution was a mess;
///       you can't evaluate the result in `evaluate` method; yet it went well
///       when just parsing and lexing.
namespace net::ancillarycat::loxograph::syntax {
class False;
class SyntaxLiteral;
class Keyword;
class String;
class Number;
class SyntaxLiteral : public utils::Printable {
public:
  constexpr SyntaxLiteral() = default;
  virtual ~SyntaxLiteral() override = default;
};
class Number : public SyntaxLiteral {
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
class String : public SyntaxLiteral, public utils::Viewable {
public:
  constexpr String() = default;
  String(const string_type &value) : value(value) {}
  String(const string_view_type value) : value(value) {}
  virtual ~String() override = default;

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
class Nil : public Keyword {
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
class True : public Keyword {
public:
  constexpr True() = default;
  virtual ~True() = default;

private:
  auto to_string_impl(const utils::FormatPolicy &format_policy) const
      -> string_type override {
    return "true"s;
  }
  auto to_string_view_impl(const utils::FormatPolicy &format_policy) const
      -> string_view_type override {
    return "true"sv;
  }
};
class False : public Keyword {
public:
  constexpr False() = default;
  virtual ~False() = default;

private:
  auto to_string_impl(const utils::FormatPolicy &format_policy) const
      -> string_type override {
    return "false"s;
  }
  auto to_string_view_impl(const utils::FormatPolicy &format_policy) const
      -> string_view_type override {
    return "false"sv;
  }
};
auto operator!(const True &) noexcept -> False;
auto operator!(const False &) noexcept -> True;
constexpr auto operator<=>(const False &, const False &) noexcept {
  return std::strong_ordering::equal;
}
constexpr auto operator<=>(const True &, const True &) noexcept {
  return std::strong_ordering::equal;
}
constexpr auto operator<=>(const Nil &, const Nil &) noexcept {
  return std::strong_ordering::equal;
}
constexpr auto operator<=>(const False &, const True &) noexcept {
  return false;
}
constexpr auto operator<=>(const True &, const False &) noexcept {
  return false;
}
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
