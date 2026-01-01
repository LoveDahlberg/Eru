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

TEST(Parser, TestExpressionPointerFailure) {

  std::vector<failureTestCase> testCases;

  testCases.push_back(
      {R"(
    int test()

    int Valinor(){
      int hello = *test()
      return 1
    }
   )",
       "Cannot perform pointer indirection on values directly being returned "
       "from function calls. Called function: 'test'."});

  testCases.push_back({R"(
    int Valinor(){
      int hello = *"test"
      return 1
    }
   )",
                       "Cannot perform pointer indirection on string literals. "
                       "Value: 'test'."});

  testCases.push_back(
      {R"(
    int Valinor(){
      int hello = *1
      return
    }
   )",
       "Cannot perform pointer indirection on integer literals. Value: '1'."});

  int order = 0;
  for (auto testCase : testCases) {
    auto item = RunParser(testCase.code, false);

    if (!item.success) {
      std::cout << "Failed for case " << std::to_string(order) << "\n";
    }

    ASSERT_TRUE(item.success);
    if (!testCase.expectedFailure.empty()) {
      EXPECT_EQ(item.result->failureDescription.front(),
                testCase.expectedFailure);
    }
  }
}