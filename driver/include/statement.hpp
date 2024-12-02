#pragma once

#include <memory>
#include <vector>
#include "config.hpp"
#include "utils.hpp"
#include "loxo_fwd.hpp"
#include "StmtVisitor.hpp"
#include "Token.hpp"

namespace net::ancillarycat::loxograph::statement {
class Stmt : public utils::Printable,
             public std::enable_shared_from_this<Stmt> {
public:
  using base_type = Stmt;
  using string_type = std::string;
  using ostream_t = std::ostream;
  using ostringstream_t = std::ostringstream;
  using token_t = Token;
  using stmt_ptr_t = std::shared_ptr<base_type>;
  using stmt_result_t = utils::Status;

public:
  virtual ~Stmt() = default;

public:
  template <typename DerivedVisitor>
    requires std::is_base_of_v<StmtVisitor, DerivedVisitor>
  auto accept(const DerivedVisitor &visitor) const {
    return accept_impl(visitor);
  }

private:
  virtual stmt_result_t accept_impl(const StmtVisitor &) const = 0;
};
class Variable : public Stmt {
public:
  Variable(Token name, std::shared_ptr<expression::Expr> initializer)
      : name(std::move(name)), initializer(std::move(initializer)) {}
  virtual ~Variable() override = default;

public:
  LOXO_CONSTEXPR_IF_NOT_MSVC auto inline has_initilizer() const noexcept
      -> bool {
    return initializer != nullptr;
  }

public:
  Token name;
  std::shared_ptr<expression::Expr> initializer = nullptr;

private:
  stmt_result_t accept_impl(const StmtVisitor &) const override;
  auto to_string_impl(const utils::FormatPolicy &format_policy) const
      -> string_type override;
};
class Print : public Stmt {
public:
  Print(std::shared_ptr<expression::Expr> value) : value(std::move(value)) {}
  virtual ~Print() override = default;

public:
  std::shared_ptr<expression::Expr> value;

private:
private:
  auto to_string_impl(const utils::FormatPolicy &format_policy) const
      -> string_type override;
  stmt_result_t accept_impl(const StmtVisitor &) const override;
};
class Expression : public Stmt {
public:
  Expression(std::shared_ptr<expression::Expr> expr) : expr(std::move(expr)) {}
  virtual ~Expression() override = default;

public:
  std::shared_ptr<expression::Expr> expr;

private:
private:
  auto to_string_impl(const utils::FormatPolicy &format_policy) const
      -> string_type override;
  stmt_result_t accept_impl(const StmtVisitor &) const override;
};
class Block : public Stmt {
public:
  Block(std::vector<std::shared_ptr<Stmt>> statements)
      : statements(std::move(statements)) {}
  virtual ~Block() override = default;

public:
  std::vector<std::shared_ptr<Stmt>> statements{};

private:
  auto to_string_impl(const utils::FormatPolicy &format_policy) const
      -> string_type override;
  stmt_result_t accept_impl(const StmtVisitor &) const override;
};
class IllegalStmt : public Stmt, utils::Viewable {
public:
  IllegalStmt(const std::string_view message_sv) : message(message_sv) {}
  explicit IllegalStmt(std::string &&message) : message(std::move(message)) {}
  virtual ~IllegalStmt() override = default;

public:
  std::string message;

private:
  auto to_string_impl(const utils::FormatPolicy &) const
      -> string_type override;
  auto to_string_view_impl(const utils::FormatPolicy &) const
      -> string_view_type override;
  stmt_result_t accept_impl(const StmtVisitor &) const override;
};
} // namespace net::ancillarycat::loxograph::statement
