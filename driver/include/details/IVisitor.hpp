#pragma once

#include <accat/auxilia/auxilia.hpp>
#include "lox_fwd.hpp"

namespace accat::lox {
/// @brief Interface for the visitor pattern
/// @interface IVisitor
class IVisitor {
public:
  using variant_type = auxilia::Variant<auxilia::Monostate,
                                        lox::evaluation::Boolean,
                                        lox::evaluation::Nil,
                                        lox::evaluation::Number,
                                        lox::evaluation::String,
                                        lox::evaluation::Function,
                                        lox::evaluation::Class,
                                        lox::evaluation::Instance>;
  using eval_result_t = auxilia::StatusOr<variant_type>;
  using string_view_type = auxilia::string_view;
};
} // namespace accat::lox
