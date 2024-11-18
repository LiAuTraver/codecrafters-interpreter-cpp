#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <print>
#include <sstream>
#include <stacktrace>
#include <string>
#include <string_view>
#include "config.hpp"
#include "lexer.hpp"
/// @note use fmt::print, fmt::println when compiling with clang-cl.exe will
/// cause some wired error: Critical error detected c0000374
/// A breakpoint instruction (__debugbreak() statement or a similar call) was
/// executed, which related to heap corruption. The program will terminate.
#if (defined(__clang__) && defined(_MSC_VER)) || (!__has_include(<fmt/core.h>))
using std::print, std::println;
#else
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
using fmt::print, fmt::println;
#endif

LOXOGRAPH_INITIALIZATION

int main(int argc, char *argv[]) {
  contract_assert(argc);
  std::filesystem::path path;
  std::string command;
#ifndef LOXOGRAPH_DEBUG_ENABLED
  if (argc != 3) {
    println(stderr, "Usage: ./your_program tokenize <filename>");
    return EXIT_FAILURE;
  }
#else
  if (argc != 3) {
    path = R"(Z:\loxograph\text.lox)";
    command = "tokenize";
  }
#endif
  else {
    path = argv[2];
    command = argv[1];
  }

  if (command == "tokenize") {
    net::ancillarycat::loxograph::lexer lexer;
    auto load_result = lexer.load(path);
    if (!load_result.ok()) {
      println("Error: {}", load_result.message().data());
    }
    auto lex_result = lexer.lex();
    if (!lex_result.ok()) {
      println("Error: {}", lex_result.message().data());
    }
    auto tokens = lexer.get_tokens();
    std::ranges::for_each(tokens, [](const auto &token) {
      println("{}", token.to_string());
    });
    return EXIT_SUCCESS;
  }

  std::cerr << "Unknown command: " << command << std::endl;
  return EXIT_FAILURE;
}
