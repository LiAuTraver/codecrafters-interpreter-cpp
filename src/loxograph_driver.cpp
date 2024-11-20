#if __has_include(<spdlog/spdlog.h>)
#include <spdlog/spdlog.h>
#endif
#if __has_include(<fmt/core.h>)
#include <fmt/core.h>
#endif
#if __has_include(<sal.h>)
#include <sal.h>
#else
#define _In_
#define _Inout_
#endif
#ifdef _WIN32
#include <io.h>
#define isatty _isatty
#define fileno _fileno
#else
#include <unistd.h>
#endif

#include <ranges>
#include <algorithm>

#include "config.hpp"
#include "lexer.hpp"
#include "status.hpp"

namespace net::ancillarycat::loxograph {
inline void inspect(std::ostringstream &oss, const utils::Status &load_result) {
  if (load_result.code() == utils::Status::kNotFoundError)
    return dbg(error, "{}", load_result.message().data());
}
// clang-format off
nodiscard_msg(loxo_main)
int loxo_main(_In_ const std::filesystem::path &path,
              _In_ const std::string_view command,
              _Inout_ std::ostringstream &oss)
// clang-format on
{
  if (command == "tokenize") {
    net::ancillarycat::loxograph::lexer lexer;
    utils::Status load_result;
#ifdef LOXOGRAPH_DEBUG_ENABLED
    dbg(info, "use `--stdin` to read from stdin.");
    if (!isatty(fileno(stdin)) && command == "--stdin") {
      dbg(info, "loxograph: reading from stdin...");
      load_result = lexer.load(std::cin);
    } else {
#endif
      dbg(info, "loxograph: reading from file...");
      dbg(info, "file path: {}", path.string());
      load_result = lexer.load(path);
#ifdef LOXOGRAPH_DEBUG_ENABLED
    }
#endif
    if (!load_result.ok()) {
      inspect(oss, load_result);
      return 65;
    }
    auto lex_result = lexer.lex();
    if (!lex_result.ok()) {
      dbg(warn, "{}", lex_result.message().data());
      return 65;
    }
    const auto tokens = lexer.get_tokens();
    std::ranges::for_each(tokens, [&oss](const auto &token) {
      if (token.type.type == TokenType::kLexError) {
        oss << token.to_string() << std::endl;
        utils::println(stderr, "{}", token.to_string());
      }
    });
    std::ranges::for_each(tokens, [&oss](const auto &token) {
      if (token.type.type != TokenType::kLexError) {
        oss << token.to_string() << std::endl;
        utils::println(stdout, "{}", token.to_string());
      }
    });
    if (!lexer.ok()) {
      dbg(error, "lexing process completed with {} error(s).", lexer.error());
      return 65;
    }
    dbg(trace, "lexing process completed successfully with no errors.");
    return 0;
  }

  dbg(error, "Unknown command: {}", command);
  return 65;
}
} // namespace net::ancillarycat::loxograph