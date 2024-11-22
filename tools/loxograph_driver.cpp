#include <print>
#if __has_include(<spdlog/spdlog.h>)
#  include <spdlog/spdlog.h>
#endif
#if __has_include(<fmt/core.h>)
#  include <fmt/core.h>
#endif
#if __has_include(<sal.h>)
#  include <sal.h>
#else
#  define _In_
#  define _Inout_
#  define _In_opt_
#endif
#ifdef _WIN32
#  include <io.h>
#  define isatty _isatty
#  define fileno _fileno
#else
#  include <unistd.h>
#endif

#include <algorithm>
#include <ranges>

#include "config.hpp"
#include "execution_context.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "status.hpp"

namespace net::ancillarycat::loxograph {
utils::Status show_msg() {
  dbg(critical, "please provide a command.");
  contract_assert(false);
  return utils::Status::kEmptyInput;
}
utils::Status onFileOperationFailed(const utils::Status &load_result) {
  dbg(error, "{}", load_result.message().data());
  return utils::Status::kPermissionDeniedError;
}
utils::Status onLexOperationFailed(const utils::Status &lex_result) {
  dbg(warn, "{}", lex_result.message().data());
  return utils::Status::kError;
}
utils::Status onLexOperationExit(const lexer &lexer) {
  dbg(error, "lexing process completed with {} error(s).", lexer.error());
  return utils::Status::kError;
}
utils::Status onCommandNotFound(const ExecutionContext &ctx) {
  dbg(error,
      "Unknown command: {}",
      ExecutionContext::command_sv(ctx.commands.front()));
  return utils::Status::kCommandNotFound;
}
void writeLexResults(ExecutionContext &ctx, const lexer::tokens_t &tokens) {
  std::ranges::for_each(tokens, [&ctx](const auto &token) {
    if (token.type.type == TokenType::kLexError) {
      ctx.output_stream << token.to_string() << std::endl;
    }
  });
  std::ranges::for_each(tokens, [&ctx](const auto &token) {
    if (token.type.type != TokenType::kLexError) {
      ctx.output_stream << token.to_string() << std::endl;
    }
  });
}
utils::StatusOr<lexer> tokenize(ExecutionContext &ctx) {
  if (ctx.input_files.size() != 1) {
    return show_msg();
  }
  lexer lexer;
  if (const utils::Status load_result = lexer.load(*ctx.input_files.cbegin());
      !load_result.ok()) {
    return onFileOperationFailed(load_result);
  }
  if (const utils::Status lex_result = lexer.lex(); !lex_result.ok()) {
    return onLexOperationFailed(lex_result);
  }
  const auto tokens = lexer.get_tokens();
  if (!lexer.ok()) {
    return onLexOperationExit(lexer);
  }
  dbg(info, "lexing process completed successfully with no errors.");
  return {std::move(lexer)};
}
utils::StatusOr<parser> parse(ExecutionContext &ctx) {
  dbg(info, "Parsing...");
  parser parser;
  parser.set_tokens(ctx.lexer->get_tokens()); //! take ownership
  auto res = parser.parse();
  if (res.ok()) {
    return {std::move(parser)};
  }
  return res;
}
// clang-format off
nodiscard_msg(loxo_main)
int loxo_main(_In_ const int argc,
              _In_opt_ char **argv, //! @note argv can be nullptr(debug mode or google test)
              _Inout_ ExecutionContext &ctx)
// clang-format on
{
  if (!argv) {
    dbg(info, "Debug mode enabled.");
  }
  if (argc == 0) {
    dbg(critical, "No arguments provided.");
    return 1;
  }
  if (ctx.commands.empty()) {
    std::println(stderr, "No command provided.");
    return 1;
  }
  if (ctx.input_files.empty()) {
    std::println(stderr, "No input files provided.");
    return 1;
  }
  lexer lexer;
  if (ctx.commands.front() == ExecutionContext::lex ||
      ctx.commands.front() == ExecutionContext::parse) {
    if (auto lex_res = tokenize(ctx)) {
      lexer = std::move(*lex_res);
    } else {
      dbg(error, "Lexing failed: {}", lex_res.message());
      return 65;
    }
  }
  if (ctx.commands.front() == ExecutionContext::lex) {
    auto tokens = lexer.get_tokens();
    writeLexResults(ctx, tokens);
    return 0;
  }
  ctx.lexer = &lexer;
  if (ctx.commands.front() == ExecutionContext::parse) {
    if (auto parse_res = parse(ctx)) {
      parser parser = std::move(*parse_res);
      auto expr = parser.get_expr();
      ASTPrinter astPrinter;
      // ASTPrinter astPrinter(std::cout);
      expr->accept(astPrinter);
      // std::cout << ctx.output_stream.str() << std::endl;
      std::cout << astPrinter.to_string() << std::endl;
    } else {
      dbg(error, "Parsing failed: {}", parse_res.message());
      return 65;
    }
  }
  return onCommandNotFound(ctx).code();
}
} // namespace net::ancillarycat::loxograph
