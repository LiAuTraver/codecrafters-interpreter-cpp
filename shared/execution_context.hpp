#pragma once

#include <cstdint>
#include <filesystem>
#include <limits>
#include <memory>
#include <ostream>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

#include <net/ancillarycat/utils/config.hpp>
#include "details/loxo_fwd.hpp"

namespace net::ancillarycat::loxo {
class LOXO_API lexer;
class LOXO_API parser;
class LOXO_API interpreter;
/// @remark why? because the forward declaration is not enough for @link
/// std::unique_ptr @endlink, also I don't want to include those implementation
/// files.
extern LOXO_API void delete_lexer_fwd(lexer *);
extern LOXO_API void delete_parser_fwd(parser *);
extern LOXO_API void delete_interpreter_fwd(interpreter *);
struct ExecutionContext;

NODISCARD_LOXO(loxo_main)
extern int loxo_main(_In_ int, _In_ char **, _Inout_ ExecutionContext &);
/// mimic the from llvm clang-driver's ToolContext
struct ExecutionContext {
  inline explicit ExecutionContext()
      : lexer(nullptr, &delete_lexer_fwd), parser(nullptr, &delete_parser_fwd),
        interpreter(nullptr, &delete_interpreter_fwd) {}
  inline ~ExecutionContext() = default;
  enum commands_t : uint16_t;
  std::filesystem::path executable_name;
  std::string_view executable_path;
  std::vector<commands_t> commands;
  std::filesystem::path execution_dir;
  std::filesystem::path tempdir;
  std::ostringstream output_stream{};
  std::ostringstream error_stream{};
  std::vector<std::filesystem::path> input_files;
  std::unique_ptr<class lexer, decltype(&delete_lexer_fwd)> lexer;
  std::unique_ptr<class parser, decltype(&delete_parser_fwd)> parser;
  std::unique_ptr<class interpreter, decltype(&delete_interpreter_fwd)>
      interpreter;
  // std::vector<std::filesystem::path> output_files;
  void addCommands(char **&);
  static ExecutionContext &inspectArgs(int, char **&, char **&);
  static std::string_view command_sv(const commands_t &);
};
namespace details {
static inline constexpr uint16_t _dummy_tag_ = 1 << 15;
static inline constexpr uint16_t _help_ = 1 << 0;
static inline constexpr uint16_t _lex_ = 1 << 1;
static inline constexpr uint16_t _parse_ = 1 << 2;
static inline constexpr uint16_t _evaluate_ = 1 << 3;
static inline constexpr uint16_t _interpret_ = 1 << 4;
static inline constexpr uint16_t _version_ = 1 << 5;
static inline constexpr uint16_t _test_ = 1 << 6;
static inline constexpr uint16_t _needs_lex_ =
    _lex_ | _parse_ | _evaluate_ | _interpret_;
static inline constexpr uint16_t _needs_parse_ =
    _parse_ | _evaluate_ | _interpret_;

/// @note MSVC has wired behavior with my enums; also the `|` operator inside
/// enum, so I made a workaround here.
#if defined(_MSC_VER) && !defined(__clang__)
static inline constexpr uint16_t _needs_evaluate_ = _evaluate_ | _dummy_tag_;
static inline constexpr uint16_t _needs_interpret_ = _interpret_ | _dummy_tag_;
#else
static inline constexpr uint16_t _needs_evaluate_ = _evaluate_;
static inline constexpr uint16_t _needs_interpret_ = _interpret_;
#endif
} // namespace details
enum ExecutionContext::commands_t : uint16_t {
  help = details::_help_,
  lex = details::_lex_,
  parse = details::_parse_,
  evaluate = details::_evaluate_,
  interpret = details::_interpret_,
  version = details::_version_,
  test = details::_test_,
  needs_lex = details::_needs_lex_,
  needs_parse = details::_needs_parse_,
  needs_evaluate = details::_needs_evaluate_,
  needs_interpret = details::_needs_interpret_,
  unknown = std::numeric_limits<uint16_t>::max(),
};

inline void ExecutionContext::addCommands(char **&argv) {
  // // currently only accept one command
  // // ctx.commands.emplace_back(*(argv + 1));
  if (std::string_view(*(argv + 1)) == "tokenize") {
    commands.emplace_back(commands_t::lex);
  } else if (std::string_view(*(argv + 1)) == "parse") {
    commands.emplace_back(commands_t::parse);
  } else if (std::string_view(*(argv + 1)) == "evaluate") {
    commands.emplace_back(commands_t::evaluate);
  } else if (std::string_view(*(argv + 1)) == "run") {
    commands.emplace_back(commands_t::interpret);
  } else if (std::string_view(*(argv + 1)) == "test") {
    commands.emplace_back(commands_t::test);
  } else if (std::string_view(*(argv + 1)) == "help") {
    commands.emplace_back(commands_t::help);
  } else if (std::string_view(*(argv + 1)) == "version") {
    commands.emplace_back(commands_t::version);
  } else
    dbg(critical, "Unknown command: {}", *(argv + 1));
}
inline ExecutionContext &
ExecutionContext::inspectArgs(const int argc, char **&argv, char **&envp) {
  static auto ctx = ExecutionContext{};
  ctx.executable_path = argv[0];
  auto path_ = std::filesystem::path(ctx.executable_path);
  ctx.executable_name = path_.filename();
  //! @note according to standard, `set_rdbuf` shall be protected;
  //! for some reason, MSVC's STL makes it public.
  //! did not compile with libstdc++.
  // ctx.output_stream.set_rdbuf(std::cout.rdbuf());
  ctx.execution_dir = std::filesystem::current_path();
  ctx.tempdir = std::filesystem::temp_directory_path();
  if (argc > 1) {
    ctx.addCommands((argv));
  }
  if (argc < 3) {
    return ctx;
  }
  // for now: ignore envp, accept only one file
  if (argc > 3) {
    dbg(error, "currently only one file is supported.");
  }
  for (auto i = 2ull; *(argv + i); ++i) {
    ctx.input_files.emplace_back(*(argv + i));
  }
  return ctx;
}
inline std::string_view ExecutionContext::command_sv(const commands_t &cmd) {
  using enum commands_t;
  using namespace std::string_view_literals;
  switch (cmd) {
  case parse:
    return "parse"sv;
  case lex:
    return "tokenize"sv;
  case evaluate:
    return "interpret"sv;
  case interpret:
    return "run"sv;
  case test:
    return "test"sv;
  case help:
    return "help"sv;
  case version:
    return "version"sv;
  default:
    return "unknown"sv;
  }
}
} // namespace net::ancillarycat::loxo
