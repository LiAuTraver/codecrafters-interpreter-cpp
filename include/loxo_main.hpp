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
#include <vector>
#include "Token.hpp"
#include "config.hpp"
#include "lexer.hpp"
#include "loxo_fwd.hpp"
#include "status.hpp"
#if __has_include(<spdlog/spdlog.h>)
#include <spdlog/spdlog.h>
#endif
#ifdef _WIN32
#include <corecrt_io.h>
#include <io.h>

#define isatty _isatty
#define fileno _fileno
#else
#include <unistd.h>
#endif
namespace net::ancillarycat::loxograph {
LOXOGRAPH_INITIALIZATION
inline void inspect(std::ostringstream &oss, const Status &load_result) {
  if (load_result.code() == Status::kNotFoundError)
    return dbg(error, "{}", load_result.message().data());
}
// template std::string loxo_main(const std::filesystem::path &,
//                                const std::string_view);
template <typename PathType = std::filesystem::path,
          typename StringType = std::string,
          typename StringViewType = std::string_view,
          typename InputStreamType = std::ifstream,
          typename OutputStringStreamType = std::ostringstream>
StringType loxo_main(const PathType &path, const StringViewType command) {

  OutputStringStreamType oss;
  if (command == "tokenize") {
    net::ancillarycat::loxograph::lexer lexer;
    Status load_result;
#ifdef LOXOGRAPH_DEBUG_ENABLED
    if (!isatty(fileno(stdin))) {
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
      return oss.str();
    }
    auto lex_result = lexer.lex();
    if (!lex_result.ok()) {
      dbg(warn, "{}", lex_result.message().data());
      return oss.str();
    }
    auto tokens = lexer.get_tokens();
    std::ranges::for_each(tokens, [&](const auto &token) {
      oss << token.to_string() << std::endl;
    });
    return oss.str();
  }

  dbg(error, "Unknown command: {}", command);
  return oss.str();
}
} // namespace net::ancillarycat::loxograph