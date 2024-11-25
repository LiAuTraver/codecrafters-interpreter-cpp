#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <source_location>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#if __has_include(<spdlog/spdlog.h>)
#  include <spdlog/spdlog.h>
#endif
#if __has_include(<fmt/core.h>)
#  include <fmt/core.h>
#  include <fmt/format.h>
#endif
#include "../shared/execution_context.hpp"

LOXOGRAPH_INITIALIZATION(trace);
using namespace net::ancillarycat;

void alterToolContext(loxograph::ExecutionContext &execution_context) {
  static auto debugInputFilePath =
      std::filesystem::path{"Z:/loxograph/examples/dynamic.lox"};
  if (execution_context.commands.empty())
    execution_context.commands.emplace_back(loxograph::ExecutionContext::interpret);
  if (execution_context.input_files.empty()) {
    if (exists(debugInputFilePath))
      execution_context.input_files.emplace_back(debugInputFilePath);
    else {
      dbg(critical, "file not found: {}", debugInputFilePath);
      LOXOGRAPH_DEBUG_BREAK
    }
  }
}
// NOLINTNEXTLINE // <-- why clang-tidy warns the main function?
int main(int argc, char **argv, char **envp) {

  auto &tool_context =
      loxograph::ExecutionContext::inspectArgs(argc, argv, envp);

  dbg_block(alterToolContext(tool_context);

            dbg(info, "Executable name: {}", tool_context.executable_name);
            dbg(info, "Executable path: {}", tool_context.executable_path);
            dbg(info,
                "Command: {}",
                tool_context.commands.empty()
                    ? "<no command provided>"
                    : loxograph::ExecutionContext::command_sv(
                          tool_context.commands.front()));
            // dbg(info, "Input files: {}", tool_context.input_files);
						// MSVC failed 										^^^^^^^^(a vec of path)
            std::ranges::for_each(
                tool_context.input_files,
                [](const auto &file) { dbg(info, "Input file: {}", file); });
            dbg(info, "Execution directory: {}", tool_context.execution_dir);
            dbg(info, "Temp directory: {}", tool_context.tempdir));

  return loxograph::loxo_main(argc, argv, tool_context);
}
