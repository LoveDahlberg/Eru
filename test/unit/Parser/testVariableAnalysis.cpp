#include "testParser.h"

TEST(Parser, TestSemanticVariableFailure) {

  std::vector<failureTestCase> testCases;

  // Assign to global variable
  testCases.push_back({R"(
   int hello = 1 
  )"});

  // Redeclaring the same variable multiple times different types
  testCases.push_back({R"(
    int hello
    bool hello
   )"});

  // Redeclaring the same variable
  testCases.push_back({R"(
    int hello
    bool hello
  )"});

  // Use of undeclared variable
  testCases.push_back({R"(
    int main() {
      hello = 1
    }
  )"});

  // Assigning wrong types string literal
  testCases.push_back({R"(
      int main(){
        int one = "one" 
      } 
    )"});

  // Assigning wrong types int literal
  testCases.push_back({R"(
      int main(){
        bool one = 1 
      } 
    )"});

  // Assigning wrong types function call
  testCases.push_back({R"(
      int something() {}
      
      int main(){
        bool one = something()
      } 
    )"});

  // Redeclare parameter
  testCases.push_back({R"(
    int main(int a) {
      int a
    }
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

TEST(Parser, TestSemanticVariableSuccess) {
  std::vector<std::string> testCases;

  // Declare and assign integer literal to local variable on one line.
  testCases.push_back(R"(
    int main() {
      int something = 1
    }
  )");

  // Declare and assign to local variable on two lines.
  testCases.push_back(R"(
    int main() {
      int something
      something = 1
    }
  )");

  // Redeclaring the same global variable in a function.
  testCases.push_back(R"(
    int something
    int main() {
      int something
    }
  )");

  // Redeclaring the same global variable in a controlflow block.
  testCases.push_back(R"(
    int something
    int main() {
      if(1){
        int something
      }
    }
  )");

  // Redeclaring the same global variable in a function and then again in a
  // controlflow block.
  testCases.push_back(R"(
    int something
    int main() {
      int something
      if(1)
      {
        int something
      }
    }
  )");

  // Redeclaring deep and assigning.
  testCases.push_back(R"(
    int something
    int main(){
      int something
      if(1){
        int something
        if(2){
          int something
          if(3){
            int something
            if(4){
              int something
              if(5){
                int something = 1
              }
            }
          }
        }
      }
    }
  )");

  // Assingment one variable to another
  testCases.push_back({R"(
    int main() {
      int hello
      int world = hello
    }
  )"});

  // Assignment to parameter
  testCases.push_back({R"(
    int main(int a) {
      a = 1
    }
  )"});

  // Assigning stringL literal, function call and identifier.
  testCases.push_back({R"(
    string something()
    int main(){
      string one = something()
      string two = "two"
      string three = two
    } 
  )"});

  for (auto testCase : testCases) {
    auto item = RunParser(testCase);
    ASSERT_TRUE(item.success);
  }
}