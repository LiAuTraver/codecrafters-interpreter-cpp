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

#include "config.hpp"

// clang-format off
namespace net::ancillarycat::loxograph {
class LOXOGRAPH_API lexer;
class LOXOGRAPH_API parser;
class LOXOGRAPH_API interpreter;
/// @remark why? because the forward declaration is not enough for @ref std::unique_ptr, also I don't want to include thoses implementation files.
extern LOXOGRAPH_API void delete_lexer_fwd(lexer *);
extern LOXOGRAPH_API void delete_parser_fwd(parser *);
extern LOXOGRAPH_API void delete_interpreter_fwd(interpreter *);
struct ExecutionContext;
LOXO_NODISCARD_MSG(loxo_main) extern
int loxo_main(_In_ int ,
              _In_ char**,
              _Inout_ ExecutionContext &);
// clang-format on
/// mimic the from llvm clang-driver's ToolContext
struct ExecutionContext {
  inline explicit ExecutionContext()
      : lexer(nullptr, &delete_lexer_fwd), parser(nullptr, &delete_parser_fwd),
        interpreter(nullptr, &delete_interpreter_fwd) {}
  inline ~ExecutionContext() = default;
  enum commands_t : uint8_t;
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
  static constexpr auto $null = std::string_view{};
  void addCommands(char **&argv);
  static ExecutionContext &inspectArgs(int argc, char **&argv, char **&envp);
  static std::string_view command_sv(const commands_t &cmd);
};
enum ExecutionContext::commands_t : uint8_t {
  help = 1 << 0,      // no command
  lex = 1 << 1,       // command "tokenize"
  parse = 1 << 2,     // command "parse"
  evaluate = 1 << 3,  // command "evaluate"
  interpret = 1 << 4, // command "run"
  version = 1 << 5,
  test = 1 << 6,
  needs_lex = lex | parse | evaluate | interpret,
  needs_parse = parse | evaluate | interpret,
  needs_evaluate = evaluate,
  needs_interpret = interpret, /// <-- interpret contains evaluate, so no need
                               /// to evaluate ^^^^^^
  unknown = std::numeric_limits<uint8_t>::max(),
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
} // namespace net::ancillarycat::loxograph
