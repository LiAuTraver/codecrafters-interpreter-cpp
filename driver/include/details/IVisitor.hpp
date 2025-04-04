#pragma once

#include <accat/auxilia/auxilia.hpp>
#include "loxo_fwd.hpp"

namespace accat::loxo {
/// @brief Interface for the visitor pattern
/// @interface IVisitor
class IVisitor {
public:
  using variant_type = auxilia::Variant<accat::auxilia::Monostate,
                                        loxo::evaluation::Boolean,
                                        loxo::evaluation::Nil,
                                        loxo::evaluation::Number,
                                        loxo::evaluation::String,
                                        loxo::evaluation::Function,
                                        loxo::evaluation::Class,
                                        loxo::evaluation::Instance>;
  using eval_result_t = auxilia::StatusOr<variant_type>;
  using string_view_type = auxilia::string_view;
};
} // namespace accat::loxo
