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
  // TODO: move or copy the token or reference it?
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
  auto to_string_impl(const utils::FormatPolicy &) const
      -> string_type override;
};
class Print : public Stmt {
public:
  constexpr Print() = default;
  explicit Print(std::shared_ptr<expression::Expr> &&value)
      : value(std::move(value)) {}
  virtual ~Print() override = default;

public:
  std::shared_ptr<expression::Expr> value = nullptr;

private:
private:
  auto to_string_impl(const utils::FormatPolicy &) const
      -> string_type override;
  stmt_result_t accept_impl(const StmtVisitor &) const override;
};
class Expression : public Stmt {
public:
  constexpr Expression() = default;
  explicit Expression(std::shared_ptr<expression::Expr> &&expr)
      : expr(std::move(expr)) {}
  virtual ~Expression() override = default;

public:
  std::shared_ptr<expression::Expr> expr = nullptr;

private:
private:
  auto to_string_impl(const utils::FormatPolicy &) const
      -> string_type override;
  stmt_result_t accept_impl(const StmtVisitor &) const override;
};
class Block : public Stmt {
public:
  constexpr Block() = default;
  explicit Block(std::vector<std::shared_ptr<Stmt>> &&statements)
      : statements(std::move(statements)) {}
  virtual ~Block() override = default;

public:
  std::vector<std::shared_ptr<Stmt>> statements{};

private:
  auto to_string_impl(const utils::FormatPolicy &) const
      -> string_type override;
  stmt_result_t accept_impl(const StmtVisitor &) const override;
};

class If : public Stmt {
public:
  constexpr If() = default;
  explicit If(std::shared_ptr<expression::Expr> &&condition,
              std::shared_ptr<Stmt> &&then_branch,
              std::shared_ptr<Stmt> &&else_branch)
      : condition(std::move(condition)), then_branch(std::move(then_branch)),
        else_branch(std::move(else_branch)) {}
  virtual ~If() = default;

public:
  std::shared_ptr<expression::Expr> condition = nullptr;
  std::shared_ptr<Stmt> then_branch = nullptr;
  std::shared_ptr<Stmt> else_branch = nullptr; // needed to set to nullptr

private:
  auto to_string_impl(const utils::FormatPolicy &) const
      -> string_type override;
  auto accept_impl(const StmtVisitor &) const -> stmt_result_t override;
};

class While : public Stmt {
public:
  constexpr While() = default;
  explicit While(std::shared_ptr<expression::Expr> &&condition,
                 std::shared_ptr<Stmt> &&body)
      : condition(std::move(condition)), body(std::move(body)) {}
  virtual ~While() = default;

public:
  std::shared_ptr<expression::Expr> condition = nullptr;
  std::shared_ptr<Stmt> body = nullptr;

private:
  auto to_string_impl(const utils::FormatPolicy &) const
      -> string_type override;
  auto accept_impl(const StmtVisitor &) const -> stmt_result_t override;
};
class For : public Stmt {
public:
  constexpr For() = default;
  explicit For(std::shared_ptr<Stmt> &&initializer,
               std::shared_ptr<expression::Expr> &&condition,
               std::shared_ptr<expression::Expr> &&increment,
               std::shared_ptr<Stmt> &&body)
      : initializer(std::move(initializer)), condition(std::move(condition)),
        increment(std::move(increment)), body(std::move(body)) {}
  virtual ~For() = default;

public:
  std::shared_ptr<Stmt> initializer = nullptr;
  std::shared_ptr<expression::Expr> condition = nullptr;
  std::shared_ptr<expression::Expr> increment = nullptr;
  std::shared_ptr<Stmt> body = nullptr;

private:
  auto to_string_impl(const utils::FormatPolicy &) const
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
