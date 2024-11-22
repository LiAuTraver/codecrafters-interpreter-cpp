#pragma once
#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_set>
#include "config.hpp"

namespace net::ancillarycat::loxograph {
/// mimic the from llvm clang-driver's ToolContext
struct ExecutionContext {
  enum commands_t : uint8_t { parse, lex, compile, run, test, help, version };
  std::filesystem::path executable_name;
  std::string_view executable_path;
  std::vector<commands_t> commands;
  std::filesystem::path execution_dir;
  std::filesystem::path tempdir;
  std::ostringstream output_stream;
  // std::filesystem::string_view triple;
  std::vector<std::filesystem::path> input_files;
  // std::vector<std::filesystem::path> output_files;
  static constexpr auto $null = std::string_view{};
  void addCommands(char **&argv) {
    // // currently only accept one command
    // // ctx.commands.emplace_back(*(argv + 1));
    if (std::string_view(*(argv + 1)) == "parse") {
      commands.emplace_back(commands_t::parse);
    } else if (std::string_view(*(argv + 1)) == "tokenize") {
      commands.emplace_back(commands_t::lex);
    } else if (std::string_view(*(argv + 1)) == "compile") {
      commands.emplace_back(commands_t::compile);
    } else if (std::string_view(*(argv + 1)) == "run") {
      commands.emplace_back(commands_t::run);
    } else if (std::string_view(*(argv + 1)) == "test") {
      commands.emplace_back(commands_t::test);
    } else if (std::string_view(*(argv + 1)) == "help") {
      commands.emplace_back(commands_t::help);
    } else if (std::string_view(*(argv + 1)) == "version") {
      commands.emplace_back(commands_t::version);
    } else
      dbg(critical, "Unknown command: {}", *(argv + 1));
  }
  static ExecutionContext &
  inspectArgs(const int argc, char **&argv, char **&envp) {
    static auto ctx = ExecutionContext{};
    ctx.executable_path = argv[0];
    auto path_ = std::filesystem::path(ctx.executable_path);
    ctx.executable_name = path_.filename();
    ctx.output_stream.set_rdbuf(std::cout.rdbuf());
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
    for (auto i = 2ull; (argv + i); ++i) {
      ctx.input_files.emplace_back(*(argv + i));
    }
    return ctx;
  }
  static std::string_view command_sv(const commands_t &cmd) {
    using enum commands_t;
    using namespace std::string_view_literals;
    switch (cmd) {
    case parse:
      return "parse"sv;
    case lex:
      return "tokenize"sv;
    case compile:
      return "compile"sv;
    case run:
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
};
} // namespace net::ancillarycat::loxograph
