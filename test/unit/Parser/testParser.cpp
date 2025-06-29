#include "testParser.h"

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
    int function(int a, bool b) [] {
      int first = 3 - "asd"
      bool second = 2 + a

      char cc = third()
      if(1 + 2) {
        int aa
        something(aa, b)
      }
      elif(second){
        somethingElse(first)
        return 2
      }
      else
      {
        int bb
        alsoSomething(a, bb, cc)
      }
      return 1
    }
    )";
  auto item = RunParser(stream);
  ASSERT_TRUE(item.success);
}