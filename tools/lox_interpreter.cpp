#include <filesystem>
#include <fstream>
#include <iostream>

#include "execution_context.hpp"

AC_SPDLOG_INITIALIZATION(lox, info);

void alterToolContext(accat::lox::ExecutionContext &execution_context) {
  static auto debugInputFilePath =
      std::filesystem::path{"Z:/lox/examples/dynamic.lox"};
  if (execution_context.commands.empty())
    execution_context.commands.emplace_back(
        accat::lox::ExecutionContext::interpret);
  if (execution_context.input_files.empty()) {
    if (exists(debugInputFilePath))
      execution_context.input_files.emplace_back(debugInputFilePath);
    else {
      dbg(critical,
          "file not found: {}."
          "you are in debug mode, please provide a file.",
          debugInputFilePath)
      AC_UTILS_DEBUG_BREAK
    }
  }
}
int main(int argc, char **argv, char **envp) {

  auto &ctx = accat::lox::ExecutionContext::inspectArgs(argc, argv, envp);

  dbg_block
  {
    alterToolContext(ctx);

    dbg(info, "Executable name: {}", ctx.executable_name);
    dbg(info, "Executable path: {}", ctx.executable_path);
    dbg(info,
        "Command: {}",
        ctx.commands.empty()
            ? "<no command provided>"
            : accat::lox::ExecutionContext::command_sv(ctx.commands.front()));
    dbg(info, "Input files: {}", ctx.input_files);
    dbg(info, "Execution directory: {}", ctx.execution_dir);
    dbg(info, "Temp directory: {}", ctx.tempdir)
  };

  return accat::lox::main(argc, argv, ctx);
}
