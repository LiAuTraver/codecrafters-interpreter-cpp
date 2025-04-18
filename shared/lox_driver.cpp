#include <algorithm>
#include <cstddef>
#include <print>
#include <ranges>
#include <utility>
#if __has_include(<spdlog/spdlog.h>)
#  include <spdlog/spdlog.h>
#endif
#if __has_include(<fmt/core.h>)
#  include <fmt/core.h>
#endif

#include <accat/auxilia/auxilia.hpp>

#include "execution_context.hpp"
#include "lexer.hpp"
#include "ASTPrinter.hpp"
#include "Environment.hpp"
#include "parser.hpp"
#include "interpreter.hpp"
#include "Resolver.hpp"

namespace accat::lox {
auxilia::Status show_msg() {
  dbg(critical, "please provide a command.")
  dbg_break
  return auxilia::InvalidArgumentError("please provide a command.");
}
auxilia::Status onFileOperationFailed(const auxilia::Status &load_result) {
  dbg(error, "{}", load_result.message().data())
  return load_result;
}
auxilia::Status onLexOperationFailed(const auxilia::Status &lex_result) {
  dbg(warn, "{}", lex_result.message().data())
  return lex_result;
}
auxilia::Status onLexOperationExit(const ExecutionContext &ctx) {
  dbg(error, "lexing process completed with {} error(s).", ctx.lexer->error())
  return {auxilia::Status::kLexError, "Lexing process completed with errors."};
}
auxilia::Status onCommandNotFound(const ExecutionContext &ctx) {
  dbg(error,
      "Unknown command: {}",
      ExecutionContext::command_sv(ctx.commands.front()))
  return auxilia::InvalidArgumentError(
      "Unknown command: "s +
      ExecutionContext::command_sv(ctx.commands.front()).data());
}
void writeLexResultsToContextStream(ExecutionContext &ctx,
                                    const lexer::tokens_t &tokens) {
  std::ranges::for_each(tokens, [&ctx](const auto &token) {
    if (token.type == TokenType::kLexError) {
      ctx.error_stream << token.to_string() << std::endl;
    }
  });
  std::ranges::for_each(tokens, [&ctx](const auto &token) {
    if (token.type != TokenType::kLexError) {
      ctx.output_stream << token.to_string() << std::endl;
    }
  });
}
auxilia::Status tokenize(ExecutionContext &ctx) {
  if (ctx.input_files.size() != 1) {
    return show_msg();
  }
  ctx.lexer.reset(new lexer);
  if (const auxilia::Status load_result =
          ctx.lexer->load(*ctx.input_files.cbegin());
      !load_result.ok()) {
    return onFileOperationFailed(load_result);
  }
  if (const auxilia::Status lex_result = ctx.lexer->lex(); !lex_result.ok()) {
    return onLexOperationFailed(lex_result);
  }
  if (!ctx.lexer->ok()) {
    return onLexOperationExit(ctx);
  }
  dbg(info, "lexing process completed successfully with no errors.")
  return {};
}
auxilia::Status parse(ExecutionContext &ctx) {
  dbg(info, "Parsing...")
  ctx.parser.reset(new parser);
  ctx.parser->set_views(ctx.lexer->get_tokens());
  auxilia::Status res;
  if (ctx.commands.front() == ExecutionContext::parse) {
    res = ctx.parser->parse(parser::kExpression);
  } else if (ctx.commands.front() & ExecutionContext::needs_evaluate) {
    res = ctx.parser->parse(parser::kExpression);
  } else if (ctx.commands.front() & ExecutionContext::needs_interpret) {
    res = ctx.parser->parse(parser::kStatement);
  } else {
    TODO("unimplemented")
  }
  dbg(info, "Parsing completed.")
  return res;
}
auxilia::Status evaluate(ExecutionContext &ctx) {
  dbg(info, "evaluating...")
  ctx.interpreter.reset(new interpreter);
  auto res = ctx.interpreter->evaluate(*ctx.parser->get_expression());
  dbg(info, "evaluation completed.")
  return std::move(res).as_status();
}
auto interpret(ExecutionContext &ctx) {
  dbg(info, "interpreting...")
  ctx.interpreter.reset(new interpreter);

  auto resolver = Resolver{*ctx.interpreter};
  if (auto res = resolver.resolve(ctx.parser->get_statements()); !res)
    // resolver error return code 65 rather than 70
    return std::make_pair(std::move(res).as_status(), 65);
  Environment::isGlobalScopeInited = false;
  auto res = ctx.interpreter->interpret(ctx.parser->get_statements());
  dbg(info, "interpretation completed.")
  if (!res)
    return std::make_pair(std::move(res).as_status(), 70);
  return std::make_pair(std::move(res).as_status(), 0);
}
void writeParseResultToContextStream(ExecutionContext &ctx) {
  expression::ASTPrinter astPrinter;
  auto res = astPrinter.evaluate(*ctx.parser->get_expression());
  contract_assert(res.ok(), "{}", res.message())
  ctx.output_stream << astPrinter.to_string();
}
void writeExprResultToContextStream(ExecutionContext &ctx) {
  // add missing newline character
  ctx.output_stream << ctx.interpreter->to_string() << '\n';
}
void writeInterpResultToContextStream(ExecutionContext &ctx) {
  // DONT add newline character
  ctx.output_stream << ctx.interpreter->to_string();
}
// clang-format off
[[nodiscard]]
int main(_In_ const int argc,
              _In_opt_ char **argv, /// @note argv can be nullptr(debug mode or google test)
              _Inout_ ExecutionContext &ctx)
// clang-format on
{
  if (!argv) {
    dbg(info, "Debug mode enabled.")
  }
  if (argc == 0) {
    dbg(critical, "No arguments provided.")
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
  if (!std::filesystem::exists(ctx.input_files.front())) {
    std::println(stderr, "File not found: {}", ctx.input_files.front().string());
    return 1;
  }
  auxilia::Status lex_result;
  if (ctx.commands.front() & ExecutionContext::needs_lex) {
    lex_result = tokenize(ctx);
  }
  if (ctx.commands.front() == ExecutionContext::lex) {
    auto tokens = ctx.lexer->get_tokens();
    writeLexResultsToContextStream(ctx, tokens);
    // codecrafter's test needs stdout and stderr
    std::cerr << ctx.error_stream.str();
    std::cout << ctx.output_stream.str() << std::endl;
    return lex_result.ok() ? 0 : 65;
  }
  auxilia::Status parse_result;
  if (ctx.commands.front() & ExecutionContext::needs_parse) {
    parse_result = parse(ctx);
  }
  if (!parse_result.ok()) {
    dbg(error, "Parsing failed: {}", parse_result.message())
    ctx.error_stream << parse_result.message() << std::endl;
    // for codecrafter's test
    if (argv)
      std::cerr << parse_result.message() << std::endl;
    return 65;
  }
  if (ctx.commands.front() == ExecutionContext::parse) {
    writeParseResultToContextStream(ctx);
    if (argv)
      std::cout << ctx.output_stream.view() << std::endl;
    return 0;
  }

  auxilia::Status evaluate_result;
  if (ctx.commands.front() & ExecutionContext::needs_evaluate) {
    evaluate_result = evaluate(ctx);
  }
  if (ctx.commands.front() == ExecutionContext::evaluate) {
    if (evaluate_result.ok()) {
      writeExprResultToContextStream(ctx);
      if (argv)
        std::cout << ctx.output_stream.view() << std::endl;
      return 0;
    } else {
      dbg(error, "Evaluation failed: {}", evaluate_result.message())
      ctx.error_stream << evaluate_result.message() << std::endl;
      // for codecrafter's test
      if (argv)
        std::cerr << evaluate_result.message() << std::endl;
      return 70;
    }
  }
  auxilia::Status interpret_result;
  int returnCode = 0;
  if (ctx.commands.front() & ExecutionContext::needs_interpret) {
    std::tie(interpret_result, returnCode) = interpret(ctx);
  }
  if (ctx.commands.front() == ExecutionContext::interpret) {
    writeInterpResultToContextStream(ctx);
    if (interpret_result.ok()) {
      if (argv)
        std::cout << ctx.output_stream.view() << std::endl;
      return 0;
    } else {
      dbg(error,
          "{} failed: {}",
          returnCode == 70 ? "Interpretation" : "Resolve process",
          interpret_result.message())
      ctx.error_stream << interpret_result.message() << std::endl;
      // for codecrafter's test
      if (argv)
        std::cout << ctx.output_stream.view();
      if (argv)
        std::cerr << ctx.error_stream.view(); // DONT add newline character
      return returnCode;
    }
  }
  return onCommandNotFound(ctx).raw_code();
}
} // namespace accat::lox
