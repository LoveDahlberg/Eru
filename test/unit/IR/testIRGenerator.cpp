// include
#include <AST/Types.h>
#include <IR/IRGenerator.h>

// getst
#include <gmock/gmock-matchers.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace IR;
using namespace AST;

TEST(IR, TestGenerateIR) {

  Top top;
  top.declarations.push_back(Declaration::VariableDeclaration(
      Types::Int(), Types::Identifier("test")));

 GenerateIR(top);

}