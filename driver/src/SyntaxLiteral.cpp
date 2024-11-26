#include "config.hpp"
#include "loxo_fwd.hpp"

#include "SyntaxLiteral.hpp"

namespace net::ancillarycat::loxograph::syntax {
Value::operator Boolean() const noexcept {
  if (dynamic_cast<const Nil *>(this)) {
    return False;
  }
  if (const auto ptr = dynamic_cast<const Boolean *>(this)) {
    return *ptr;
  }
  return True;
}
Boolean Value::operator!() const noexcept {
  return Boolean{!this->operator Boolean() };
}
Boolean &Boolean::operator=(const Boolean &value) {
  this->value = value.value;
  return *this;
}
Boolean &Boolean::operator=(Boolean &&value) noexcept {
  this->value = value.value;
  return *this;
}
auto Boolean::operator==(const Boolean &rhs) const -> bool {
  return value == rhs.value;
}
auto Boolean::operator<=>(const Boolean &rhs) const -> std::strong_ordering {
  return value <=> rhs.value;
}
auto Boolean::to_string_impl(const utils::FormatPolicy &format_policy) const
    -> string_type {
  contract_assert(value.has_value());
  return value.value() ? "true"s : "false"s;
}
auto Boolean::to_string_view_impl(
    const utils::FormatPolicy &format_policy) const -> string_view_type {
  contract_assert(value.has_value());
  return value.value() ? "true"sv : "false"sv;
}
auto Nil::to_string_impl(const utils::FormatPolicy &format_policy) const
    -> string_type {
  return "nil"s;
}
auto Nil::to_string_view_impl(const utils::FormatPolicy &format_policy) const
    -> string_view_type {
  return "nil"sv;
}
Boolean String::operator==(const String &rhs) const {
  return {value == rhs.value};
}
Boolean String::operator!=(const String &rhs) const {
  return {value != rhs.value};
}
auto String::to_string_impl(const utils::FormatPolicy &format_policy) const
    -> string_type {
  return value;
}
auto String::to_string_view_impl(const utils::FormatPolicy &format_policy) const
    -> string_view_type {
  return value;
}
auto Number::to_string_impl(const utils::FormatPolicy &format_policy) const
    -> string_type {
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
} // namespace net::ancillarycat::loxograph::syntax
