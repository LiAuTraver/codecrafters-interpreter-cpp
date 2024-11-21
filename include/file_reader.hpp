#pragma once

#include <fstream>

#include "config.hpp"
#include "loxo_fwd.hpp"

namespace net::ancillarycat::utils {
class file_reader {
public:
  using path_t = path;
  using string_t = string;
  using ifstream_t = ifstream;
  using ostringstream_t = ostringstream;

public:
  inline explicit
  // constexpr // clang 18.1 failed
   file_reader(path_t path_)
      : filePath(std::move(path_)) {}
  inline constexpr ~file_reader()  = default;

public:
  nodiscard_msg(path_t) inline string_t get_contents() const {
    ifstream_t file(filePath);
    if (not file) {
      return string_t();
    }
    ostringstream_t buffer;
    buffer << file.rdbuf();
    return buffer.str();
  }
  nodiscard_msg(path_t) inline path_t filepath() const  { return filePath; }

private:
  const path_t filePath;
};
} // namespace net::ancillarycat::utils