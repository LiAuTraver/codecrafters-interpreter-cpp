#include <print>
#include "expr.hpp"
#include "test_env.hpp"


TEST(AST, dummy) {
  // 2 + 3
  auto lit2 = std::make_shared<Literal>(Token{TokenType::kNumber, "2"sv, 2.0l});
  auto lit3 = std::make_shared<Literal>(Token{TokenType::kNumber, "3"sv, 3.0l});
  auto binary =
      std::make_shared<Binary>(Token{TokenType::kPlus, "+"sv, '+'}, lit2, lit3);
  auto grouping = std::make_shared<Grouping>(binary);
  auto astPrinter = ASTPrinter(std::cout);
  grouping->accept(astPrinter);
  std::println("{}", astPrinter.to_string());
}
