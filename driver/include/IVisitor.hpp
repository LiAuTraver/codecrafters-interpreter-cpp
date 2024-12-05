#pragma once

#include "loxo_fwd.hpp"
namespace net::ancillarycat::utils {
class IVisitor : virtual public Printable {
public:
  using variant_type = Variant<::net::ancillarycat::utils::Monostate,
                               loxo::evaluation::Boolean,
                               loxo::evaluation::Nil,
                               loxo::evaluation::Number,
                               loxo::evaluation::String,
                               loxo::evaluation::Callable,
                               loxo::evaluation::Error>;
  using eval_result_t = variant_type;
  using stmt_result_t = Status;
  using string_view_type = utils::Viewable::string_view_type;
};
} // namespace net::ancillarycat::utils
