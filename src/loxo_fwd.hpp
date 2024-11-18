#pragma once

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace net::ancillarycat::utils {
class TokenType;
class Status;
class file_reader;
using StringType = std::string;
using StringViewType = std::string_view;
using PathType = std::filesystem::path;
using BooleanType = bool;
using StatusType = Status;
using InputStreamType = std::ifstream;
using OutputStringStreamType = std::ostringstream;
using PathType = std::filesystem::path;
} // namespace net::ancillarycat::utils
namespace net::ancillarycat::loxograph {
class lexer;
class Token;
using namespace ::net::ancillarycat::utils;
} // namespace net::ancillarycat::loxograph
