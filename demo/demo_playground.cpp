#include <fmt/base.h>
#include <string>
using namespace std::literals;
auto str = "Global"sv;
void p(){
  fmt::println("{}", str);
}

void closure(){
  p(); // 1
  auto str = "Block"sv;
  fmt::println("{}", str); // 2
  p(); // 1
}

int main(){
  closure();
  return 0;
}