
#include <IR/IRGenerator.h>
#include <iostream>

namespace IR {

void GenerateIR(AST::Top top) {
  auto values = top.codegen();
  for (auto value : values) {
    std::cout << "GenerateIR\n '" << value << "'";
  }
}

} // namespace IR