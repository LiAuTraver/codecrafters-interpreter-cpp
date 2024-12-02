#pragma once
#include <variant>
#include <cmath>

#include "config.hpp"
#include "utils.hpp"
#include "loxo_fwd.hpp"
#include "status.hpp"

namespace net::ancillarycat::loxograph::statement {
class StmtVisitor : virtual public utils::VisitorBase {
public:
  template <typename DerivedStmt>
    requires std::is_base_of_v<Stmt, DerivedStmt>
  auto visit(const DerivedStmt &stmt) const {
    return visit_impl(stmt);
  }
  auto execute(const Stmt &stmt) const { return execute_impl(stmt); }
private:
  virtual utils::Status visit_impl(const Variable &) const = 0;
  virtual utils::Status visit_impl(const Print &) const = 0;
  virtual utils::Status visit_impl(const Expression &) const = 0;
  virtual utils::Status visit_impl(const Block &) const = 0;
  virtual utils::Status visit_impl(const IllegalStmt &) const = 0;
  virtual utils::Status execute_impl(const Stmt &) const = 0;
};

class DummyVisitor : public StmtVisitor {
private:
  auto to_string_impl(const utils::FormatPolicy &format_policy) const
      -> string_type override {
    return "dummy visitor";
  }
  utils::Status visit_impl(const Variable &) const override { return {}; }
  utils::Status visit_impl(const Print &) const override { return {}; }
  utils::Status visit_impl(const Expression &) const override { return {}; }
  utils::Status execute_impl(const Stmt &) const override { return {}; }
  utils::Status visit_impl(const IllegalStmt &) const override { return {}; }
  utils::Status visit_impl(const Block &) const override { return {}; }
} inline static const _dummy_visitor;
} // namespace net::ancillarycat::loxograph::statement
