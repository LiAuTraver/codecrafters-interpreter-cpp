#include "test_env.hpp"

namespace {
std::pair<int, std::string> get_result(const auto &filepath) {
  ExecutionContext ec;
  ec.commands.push_back(ExecutionContext::interpret);
  ec.input_files.push_back(filepath);
  auto exec = loxo_main(3, nullptr, ec);
  if (exec == 0) {
    return {exec, ec.output_stream.str()};
  }
  return {exec, ec.error_stream.str() + ec.output_stream.str()};
}
} // namespace

TEST(interpret,print){
  const auto path = R"(Z:\loxograph\examples\inter.stmt1.lox)";
  auto [callback, str] = get_result(path);
  EXPECT_EQ(str, "Hello, World!\n42\ntrue\n36\n");
  EXPECT_EQ(callback,0);
}