#include <stdio.h>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>
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
#include "config.hpp"
#include "loxo_fwd.hpp"
// clang-format off
namespace net::ancillarycat::loxograph {
LOXOGRAPH_INITIALIZATION(trace)
nodiscard_msg(loxo_main) extern
int loxo_main(_In_ const std::filesystem::path &,
              _In_ const std::string_view,
              _Inout_ std::ostringstream &);
} // namespace net::ancillarycat::loxograph
// clang-format on
int main(int argc, char **argv, char **envp) {
  contract_assert(argc);
  std::filesystem::path path;
  std::string command;
#ifndef LOXOGRAPH_DEBUG_ENABLED
  if (argc != 3) {
    fprintf(stderr, "Usage: ./your_program tokenize <filename>");
    return EXIT_FAILURE;
  }
#else
  if (argc == 2) {
    path = argv[1];
    command = "tokenize";
  } else if (argc != 3) {
    path = R"(Z:\loxograph\templates\dynamic.lox)";
    command = "tokenize";
  }
#endif
  else {
    path = argv[2];
    command = argv[1];
  }
  std::ostringstream oss;
  const auto result =
      net::ancillarycat::loxograph::loxo_main(path, command, oss);
  //  print("{}", oss.str());
  return result;
}
