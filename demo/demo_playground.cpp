#include <fmt/base.h>
int a = 1;
void p(){
  fmt::println("{}", a);
}

void closure(){
  p(); // 1
  int a = 2;
  p(); // 1
}

int main(){
  closure();
  return 0;
}