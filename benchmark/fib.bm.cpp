#include <benchmark/benchmark.h>
#include <filesystem>
#include "test_env.hpp"
namespace {
auto get_result(auto &&filepath) {
  ExecutionContext ec;
  ec.commands.push_back(ExecutionContext::interpret);
  ec.input_files.push_back(filepath);
  auto exec = loxo_main(3, nullptr, ec);
  return exec ? std::make_pair(exec,
                               ec.output_stream.str() + ec.error_stream.str())
              : std::make_pair(exec, ec.output_stream.str());
}
} // namespace
static auto fibStr = R"(
fun fib(n){
  if (n <= 1) return n;
  return fib(n - 1) + fib(n - 2);
}
)"s;
static void BM_Fib(benchmark::State &state) {
  auto i = static_cast<unsigned>(state.range(0));
  for (auto _ : state) {
    auto fibCode = fibStr + "print fib(" + fmt::to_string(i) + ");";
    auto currentPath = current_path();
    auto filePath =
        currentPath / "fib"s.append(fmt::to_string(i)).append(".lox");
    auto f = std::fstream(filePath, std::ios::out);
    f << fibCode;
    f.close();
    auto [_2, str] = get_result(filePath);
    std::filesystem::remove(filePath);
  }
}

BENCHMARK(BM_Fib)->DenseRange(0, 20);

BENCHMARK_MAIN();
