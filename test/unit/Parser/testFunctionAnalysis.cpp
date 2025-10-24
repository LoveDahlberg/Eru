#include "testParser.h"

TEST(Parser, TestSemanticFunctionFailure) {

  std::vector<failureTestCase> testCases;

  // Declaring function inside another function.
  testCases.push_back({R"(
    int something(int a) [] {
      int somethingElse(int b)
    }
  )",
                       "ParseVaribleAndMaybeAssignment: data type: newline "
                       "does not follow a variable assignment."});

  // Redeclaring function with different parameters.
  testCases.push_back({
      R"(
    int something(int a, bool g)
    int something(bool c, int h)
  )",
      "addFunction: external function declaration with the same name but with "
      "different parameters already declared."});

  // Redefining function with different parameters.
  testCases.push_back(
      {R"(
    int something(int a, bool g) [] {}
    int something(bool c, int h) [] {}
  )",
       "addFunction: Function defined multiple times in the same "
       "compilation unit."});

  // Definition of function with different parameters compared with
  // declaration.
  testCases.push_back({R"(
    int something(int a, bool g)
    int something(bool c, int h) [] {}
  )",
                       "addFunction: function was previously declared as an "
                       "external, but is now also defined."});

  // Redefining functions.
  testCases.push_back({R"(
    int something(int a) [] {}
    int something(int a) [] {}
  )",
                       "addFunction: Function defined multiple times in the "
                       "same compilation unit."});

  // Redeclaring function with different return type.
  testCases.push_back({R"(
    int something(int a, bool g)
    bool something(int a, bool g)
  )",
                       "addFunction: Function with the same name but with a "
                       "different type already declared."});

  // Calling something that is never declared later.
  testCases.push_back({R"(
    int main(int a) [] {
      something(a)
    }
  )"});

  // Calling something that is declared with wrong parameters.
  testCases.push_back({R"(
    int something(bool a)

    int main(int a) [] {
      something(a)
    }
  )"});

  // Calling something that is declared later with a wrong parameters.
  testCases.push_back({R"(
      int main(int a) [] {
        something(a)
      }
      int something(bool a)
    )"});

  // Calling something that is declared later with a wrong return type.
  testCases.push_back({R"(
    int main(int a) [] {
      bool b = something(a)
    }
    int something(int a)
  )"});

  for (auto testCase : testCases) {
    auto item = RunParser(testCase.code, false);
    ASSERT_TRUE(item.success);
    if (!testCase.expectedFailure.empty()) {
      EXPECT_EQ(item.result->failureDescription.front(),
                testCase.expectedFailure);
    }
  }
}

TEST(Parser, TestSemanticFunctionSuccess) {
  std::vector<std::string> testCases;

  // Declaring function thats already been declared.
  testCases.push_back(R"(
    int something(int a)
    int something(int a)
  )");

  // Calling something that is declared later
  testCases.push_back(R"(
      int main(int a) [] {
        something(1)
      }
      int something(int a)
    )");

  // Calling something that is defined later
  testCases.push_back(R"(
      int main(int a) [] {
        something(1)
      }
      int something(int a) [] {}
    )");

  // Call function as parameter to other function. Once when we don't need the
  // return type, once when we do.
  testCases.push_back(R"(
      int something(bool a) [] {}
      bool somethingElse(int a) [] {}


      int main(int a) [] {
        something(somethingElse(a))
        return something(somethingElse(a))
      }
    )");

  for (auto testCase : testCases) {
    auto item = RunParser(testCase);
    ASSERT_TRUE(item.success);
  }
}