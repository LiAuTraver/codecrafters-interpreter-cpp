#pragma once

#include <accat/auxilia/auxilia.hpp>
#include "loxo_fwd.hpp"

namespace accat::auxilia {
/// @brief Interface for the visitor pattern
/// @interface IVisitor
class IVisitor  {
public:
  using variant_type = Variant<accat::auxilia::Monostate,
                               loxo::evaluation::Boolean,
                               loxo::evaluation::Nil,
                               loxo::evaluation::Number,
                               loxo::evaluation::String,
                               loxo::evaluation::Callable>;
  using eval_result_t = StatusOr<variant_type>;
  using string_view_type = auxilia::string_view;
};
} // namespace accat::auxilia
