#pragma once
#include "config.hpp"
#include "loxo_fwd.hpp"
namespace net::ancillarycat::utils {
template <typename PathType,
          typename StringType,
          typename InputStreamType,
          typename OutputStringStreamType>
class file_reader {
public:
  using path_t = PathType;
  using string_t = StringType;
  using ifstream_t = InputStreamType;
  using ostringstream_t = OutputStringStreamType;

public:
  inline explicit constexpr file_reader(path_t path) NOEXCEPT
      : filePath(std::move(path)) {}
  inline constexpr ~file_reader() NOEXCEPT = default;

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
  nodiscard_msg(path_t) inline path_t path() const NOEXCEPT { return filePath; }

private:
  const path_t filePath;
};
} // namespace net::ancillarycat::utils