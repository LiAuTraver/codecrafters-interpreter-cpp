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
#include "status.hpp"

namespace net::ancillarycat::loxograph {
int show_msg() {
  fmt::print(stderr, "currently only one file is supported.\n");
  return 65;
}
int onFileOperationFailed(const utils::Status &load_result) {
  dbg(error, "{}", load_result.message().data());
  return 65;
}
int onLexOperationFailed(const utils::Status &lex_result) {
  dbg(warn, "{}", lex_result.message().data());
  return 65;
}
int onLexOperationExit(const lexer &lexer) {
  dbg(error, "lexing process completed with {} error(s).", lexer.error());
  return 65;
}
int onCommandNotFound(const ExecutionContext &ctx) {
  dbg(error, "Unknown command: {}", ExecutionContext::command_sv(ctx.commands.front()));
  return 65;
}
void writeLexResults(ExecutionContext &ctx, const lexer::tokens_t& tokens) {
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
int tokenize(ExecutionContext &ctx) {
  if (ctx.input_files.size() != 1) {
    return show_msg();
  }
  lexer lexer;
  if (const utils::Status load_result = lexer.load(*ctx.input_files.begin());
      !load_result.ok()) {
    return onFileOperationFailed(load_result);
  }
  if (const utils::Status lex_result = lexer.lex(); !lex_result.ok()) {
    return onLexOperationFailed(lex_result);
  }
  const auto tokens = lexer.get_tokens();
  if (ctx.commands.front() == ExecutionContext::lex)
    writeLexResults(ctx, tokens);
  if (!lexer.ok()) {
    return onLexOperationExit(lexer);
  }
  dbg(trace, "lexing process completed successfully with no errors.");
  return 0;
}
int parse(ExecutionContext &ctx) {
  // TODO(implement parser);
	// parser parser;
	// ...
  dbg(info, "Parsing...");
  return 0;
}
// clang-format off
nodiscard_msg(loxo_main)
int loxo_main(_In_ const int argc,
              _In_opt_ char **argv, //! @note argv can be nullptr(debug mode or google test)
              _Inout_ ExecutionContext &ctx)
// clang-format on
{
  if (!argv) {
    // nothing now
  }
  if (argc == 0) {
    return show_msg();
  }
  if (ctx.commands.front() == ExecutionContext::lex) {
    return tokenize(ctx);
  }
  if (ctx.commands.front() == ExecutionContext::parse) {
    // todo: implement parser
    tokenize(ctx);
    return parse(ctx);
  }
  return onCommandNotFound(ctx);
}
} // namespace net::ancillarycat::loxograph
