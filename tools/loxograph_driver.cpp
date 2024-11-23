#include <cstddef>
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
utils::Status onLexOperationExit(const ExecutionContext &ctx) {
  dbg(error, "lexing process completed with {} error(s).", ctx.lexer->error());
  return utils::Status::kError;
}
utils::Status onCommandNotFound(const ExecutionContext &ctx) {
  dbg(error,
      "Unknown command: {}",
      ExecutionContext::command_sv(ctx.commands.front()));
  return utils::Status::kCommandNotFound;
}
void writeLexResultsToContextStream(ExecutionContext &ctx, const lexer::tokens_t &tokens) {
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
utils::Status tokenize(ExecutionContext &ctx) {
  if (ctx.input_files.size() != 1) {
    return show_msg();
  }
  // lexer lexer;
	ctx.lexer = std::make_shared<class lexer>();
  if (const utils::Status load_result = ctx.lexer->load(*ctx.input_files.cbegin());
      !load_result.ok()) {
    return onFileOperationFailed(load_result);
  }
  if (const utils::Status lex_result = ctx.lexer->lex(); !lex_result.ok()) {
    return onLexOperationFailed(lex_result);
  }
  const auto tokens = ctx.lexer->get_tokens();
  if (!ctx.lexer->ok()) {
    return onLexOperationExit(ctx);
  }
  dbg(info, "lexing process completed successfully with no errors.");
  return utils::Status::kOkStatus;
}
utils::Status parse(ExecutionContext &ctx) {
  dbg(info, "Parsing...");
	ctx.parser = std::make_shared<class parser>();
  ctx.parser->set_views(ctx.lexer->get_tokens());
  auto res = ctx.parser->parse();
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
	utils::Status lex_result;
  if (ctx.commands.front() == ExecutionContext::lex ||
      ctx.commands.front() == ExecutionContext::parse) {
    lex_result = tokenize(ctx);
  }
  if (ctx.commands.front() == ExecutionContext::lex) {
    auto tokens = ctx.lexer->get_tokens();
    writeLexResultsToContextStream(ctx, tokens);
    std::cout << ctx.output_stream.str() << std::endl;
    return lex_result.ok() ? 0 : 65;
  }
  if (ctx.commands.front() == ExecutionContext::parse) {
    if (auto parse_result = parse(ctx); parse_result.ok()) {
      auto expr = ctx.parser->get_expr();
      ASTPrinter astPrinter;
      expr->accept(astPrinter);
      std::cout << ctx.output_stream.str() << std::endl;
      // std::cout << astPrinter.to_string() << std::endl;
      return 0;
    } else {
      dbg(error, "Parsing failed: {}", parse_result.message());
      return 65;
    }
  }
  return onCommandNotFound(ctx).code();
}
} // namespace net::ancillarycat::loxograph
