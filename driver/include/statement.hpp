#pragma once

#include <memory>
#include <vector>

#include "details/loxo_fwd.hpp"

#include "Token.hpp"

namespace net::ancillarycat::loxo::statement {
class Stmt : public utils::Printable,
             public std::enable_shared_from_this<Stmt> {
public:
  using base_type = Stmt;
  using string_type = std::string;
  using ostream_t = std::ostream;
  using ostringstream_t = std::ostringstream;
  using token_t = Token;
  using stmt_ptr_t = std::shared_ptr<base_type>;
  using expr_ptr_t = std::shared_ptr<expression::Expr>;
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
  Variable(Token name, expr_ptr_t initializer)
      : name(std::move(name)), initializer(std::move(initializer)) {}
  virtual ~Variable() override = default;

public:
  AC_UTILS_CONSTEXPR_IF_NOT_MSVC auto inline has_initilizer() const noexcept
      -> bool {
    return initializer != nullptr;
  }

public:
  token_t name;
  expr_ptr_t initializer;

private:
  auto accept_impl(const StmtVisitor &) const -> stmt_result_t override;
  auto
  to_string_impl(const utils::FormatPolicy &) const -> string_type override;
};
class Print : public Stmt {
public:
  constexpr Print() = default;
  explicit Print(expr_ptr_t &&value) : value(std::move(value)) {}
  virtual ~Print() override = default;

public:
  expr_ptr_t value;

private:
private:
  auto
  to_string_impl(const utils::FormatPolicy &) const -> string_type override;
  auto accept_impl(const StmtVisitor &) const -> stmt_result_t override;
};
class Expression : public Stmt {
public:
  constexpr Expression() = default;
  explicit Expression(expr_ptr_t &&expr) : expr(std::move(expr)) {}
  virtual ~Expression() override = default;

public:
  expr_ptr_t expr;

private:
private:
  auto
  to_string_impl(const utils::FormatPolicy &) const -> string_type override;
  auto accept_impl(const StmtVisitor &) const -> stmt_result_t override;
};
class Block : public Stmt {
public:
  constexpr Block() = default;
  explicit Block(std::vector<stmt_ptr_t> &&statements)
      : statements(std::move(statements)) {}
  virtual ~Block() override = default;

public:
  std::vector<stmt_ptr_t> statements;

private:
  auto
  to_string_impl(const utils::FormatPolicy &) const -> string_type override;
  auto accept_impl(const StmtVisitor &) const -> stmt_result_t override;
};

class If : public Stmt {
public:
  constexpr If() = default;
  explicit If(expr_ptr_t &&condition,
              stmt_ptr_t &&then_branch,
              stmt_ptr_t &&else_branch)
      : condition(std::move(condition)), then_branch(std::move(then_branch)),
        else_branch(std::move(else_branch)) {}
  virtual ~If() = default;

public:
  expr_ptr_t condition;
  stmt_ptr_t then_branch;
  stmt_ptr_t else_branch; // needed to set to nullptr

private:
  auto
  to_string_impl(const utils::FormatPolicy &) const -> string_type override;
  auto accept_impl(const StmtVisitor &) const -> stmt_result_t override;
};

class While : public Stmt {
public:
  constexpr While() = default;
  explicit While(expr_ptr_t &&condition, stmt_ptr_t &&body)
      : condition(std::move(condition)), body(std::move(body)) {}
  virtual ~While() = default;

public:
  expr_ptr_t condition;
  stmt_ptr_t body;

private:
  auto
  to_string_impl(const utils::FormatPolicy &) const -> string_type override;
  auto accept_impl(const StmtVisitor &) const -> stmt_result_t override;
};
class For : public Stmt {
public:
  constexpr For() = default;
  explicit For(stmt_ptr_t &&initializer,
               expr_ptr_t &&condition,
               expr_ptr_t &&increment,
               stmt_ptr_t &&body)
      : initializer(std::move(initializer)), condition(std::move(condition)),
        increment(std::move(increment)), body(std::move(body)) {}
  virtual ~For() = default;

public:
  stmt_ptr_t initializer;
  expr_ptr_t condition;
  expr_ptr_t increment;
  stmt_ptr_t body;

private:
  auto
  to_string_impl(const utils::FormatPolicy &) const -> string_type override;
  auto accept_impl(const StmtVisitor &) const -> stmt_result_t override;
};
class Function : public Stmt {
public:
  constexpr Function() = default;
  explicit Function(token_t &&name,
                    std::vector<token_t> &&parameters,
                    std::vector<stmt_ptr_t> &&body)
      : name(std::move(name)), parameters(std::move(parameters)),
        body(std::move(body)) {}
  virtual ~Function() = default;

public:
  token_t name;
  std::vector<token_t> parameters;
  Block body;

private:
  auto
  to_string_impl(const utils::FormatPolicy &) const -> string_type override;
  auto accept_impl(const StmtVisitor &) const -> stmt_result_t override;
};

class Return : public Stmt {
public:
  constexpr Return() = default;
  explicit Return(expr_ptr_t &&value) : value(std::move(value)) {}
  virtual ~Return() = default;

public:
  expr_ptr_t value;

private:
  auto
  to_string_impl(const utils::FormatPolicy &) const -> string_type override;
  auto accept_impl(const StmtVisitor &) const -> stmt_result_t override;
};
} // namespace net::ancillarycat::loxo::statement
