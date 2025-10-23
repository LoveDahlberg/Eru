#include "testParser.h"

#include <llvm/IR/Verifier.h>

TEST(Parser, TestDeclarationsSuccess) {
  std::string stream = R"(
    int first
    char second

    int fourth()
    int bruh(char one)
    uint32 brotha(sint32 hello, bool world)
    )";

  auto item = RunParser(stream);
  ASSERT_TRUE(item.success);

  EXPECT_EQ(item.astContext.compilationUnit->compilationUnitItems.size(), 5);
}

TEST(Parser, TestFunctionBody) {
  std::string stream = R"(
    // Considered extern
    char third()
    int something(int aa, string b)
    int somethingElse(int first)
    char alsoSomething(int a, int bb, string cc)

    int function(int a, bool b) [] {
      int first = 3 - 1
      string second = "a" + "b"

      char cc = third()
      if(1 + 2) {
        int aa
        something(aa, second)
      }
      elif(second){
        somethingElse(first)
        return 2
      }
      else
      {
        int bb
        alsoSomething(first, bb, second)
      }
      return 1
    }
    )";

  auto item = RunParser(stream);
  ASSERT_TRUE(item.success);
}

// TODO: This are just temporary testing functions used to iron out some bugs.
// They should not be placed here in the unit parser, as it tests the parser +
// the IRGenerator together.
// I'm unsure if this should just be a lit test or if these tests should be
// added to like a Compiler test.. but thats essentially the whole program at
// this point.

#include <IR/IRGenerator.h>

#include <gmock/gmock-matchers.h>
#include <gmock/gmock.h>

TEST(Parser, testSmallFunctionBody) {
  std::string stream = R"(
  int something(int a) []

int main(int a) [] {
  int b = something(1)
  return b + a
}
)";

  auto item = RunParser(stream);
  ASSERT_TRUE(item.success);

  auto ctx = llvm::LLVMContext();
  auto module = llvm::Module("default-module", ctx);

  auto generator = IR::IRGenerator(module);

  EXPECT_THAT(generator.Walk(item.astContext),
              testing::Each(testing::NotNull()));
  EXPECT_FALSE(llvm::verifyModule(module, &llvm::errs()));
}

// Fix so that variables are found also looking at the parent scopes.
TEST(Parser, testSmallFunctionBody2) {
  std::string stream = R"(
  int something(int a) []

  int d

int main(int a) [] {
  int b = something(a)
  int g = b + a

  if(1) {
    d = g + 1
  }

  return g
}
)";

  auto item = RunParser(stream);
  ASSERT_TRUE(item.success);

  auto ctx = llvm::LLVMContext();
  auto module = llvm::Module("default-module", ctx);

  auto generator = IR::IRGenerator(module);

  EXPECT_THAT(generator.Walk(item.astContext),
              testing::Each(testing::NotNull()));
  EXPECT_FALSE(llvm::verifyModule(module, &llvm::errs()));
}

TEST(Parser, testSmallFunctionBody3) {
  std::string stream = R"(
int external(int input) []

int main(int input) [] {
  int response = external(input)

  return input + response
}
)";

  auto item = RunParser(stream);
  ASSERT_TRUE(item.success);

  auto ctx = llvm::LLVMContext();
  auto module = llvm::Module("default-module", ctx);

  auto generator = IR::IRGenerator(module);

  EXPECT_THAT(generator.Walk(item.astContext),
              testing::Each(testing::NotNull()));
  EXPECT_FALSE(llvm::verifyModule(module, &llvm::errs()));
}
