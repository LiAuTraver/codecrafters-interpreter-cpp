#include "config.hpp"
#include "loxo_fwd.hpp"
namespace net::ancillarycat::utils {
/// @brief a simple file reader that reads the contents of a file
/// @tparam PathType the path type
/// @tparam StringType the string type
/// @tparam InputStreamType the input stream type
/// @tparam OutputStringStreamType the output string stream type
/// @note the file reader is not thread-safe, and will consume a lot of memory
/// if the file is too big. @todo here.
// template <typename PathType = std::filesystem::path,
//           typename StringType = std::string,
//           typename InputStreamType = std::ifstream,
//           typename OutputStringStreamType = std::ostringstream>
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
  [[nodiscard]] inline string_t get_contents() const {
    ifstream_t file(filePath);
    if (not file) {
      return string_t();
    }
    ostringstream_t buffer;
    buffer << file.rdbuf();
    return buffer.str();
  }
  [[nodiscard]] inline path_t path() const NOEXCEPT { return filePath; }

private:
  const path_t filePath;
};
} // namespace net::ancillarycat::utils