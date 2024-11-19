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

#include "config.hpp"
#include "lexer.hpp"
#include "loxo_fwd.hpp"
#include "loxo_main.hpp"

int main(int argc, char *argv[]) {
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
    path = R"(Z:\loxograph\templates\empty.lox)";
    command = "tokenize";
  }
#endif
  else {
    path = argv[2];
    command = argv[1];
  }

  auto str = net::ancillarycat::loxograph::loxo_main(path, command);
  print("{}", str);
  return EXIT_SUCCESS;
}
