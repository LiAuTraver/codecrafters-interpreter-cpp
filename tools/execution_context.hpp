#pragma once
#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_set>
#include "config.hpp"

namespace net::ancillarycat::loxograph {
/// mimic the from llvm clang-driver's ToolContext
struct ExecutionContext {
  std::filesystem::path executable_name;
  std::string_view executable_path;
  std::vector<std::string_view> commands;
  std::filesystem::path execution_dir;
  std::filesystem::path tempdir;
  std::ostringstream output_stream;
  // std::filesystem::string_view triple;
  std::vector<std::filesystem::path> input_files;
  // std::vector<std::filesystem::path> output_files;
  static constexpr auto $null = std::string_view{};
  static ExecutionContext &inspectArgs(const int argc, char **&argv,
                                       char **&envp) {
    static auto ctx = ExecutionContext{};
    ctx.executable_path = argv[0];
    auto path_ = std::filesystem::path(ctx.executable_path);
    ctx.executable_name = path_.filename();
    ctx.output_stream.set_rdbuf(std::cout.rdbuf());
    ctx.execution_dir = std::filesystem::current_path();
    ctx.tempdir = std::filesystem::temp_directory_path();
    if (argc > 1){
    // currently only accept one command
      ctx.commands.emplace_back(*(argv + 1));
    }
    if (argc < 3) {
      return ctx;
    }
    for (auto i = 2ull; (argv + i); ++i) {
      ctx.input_files.emplace_back(*(argv + i));
    }
    // for now: ignore envp, accept only one file
    if (argc > 3) {
      dbg(error, "currently only one file is supported.");
    }
    return ctx;
  }
};
} // namespace net::ancillarycat::loxograph