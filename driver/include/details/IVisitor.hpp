#pragma once

#include <net/ancillarycat/utils/Status.hpp>
#include "loxo_fwd.hpp"

namespace net::ancillarycat::utils {
/// @brief Interface for the visitor pattern
/// @interface IVisitor
class IVisitor : public Printable {
public:
  using variant_type = Variant<::net::ancillarycat::utils::Monostate,
                               loxo::evaluation::Boolean,
                               loxo::evaluation::Nil,
                               loxo::evaluation::Number,
                               loxo::evaluation::String,
                               loxo::evaluation::Callable>;
  using eval_result_t = StatusOr<variant_type>;
  using string_view_type = utils::Viewable::string_view_type;
};
} // namespace net::ancillarycat::utils
