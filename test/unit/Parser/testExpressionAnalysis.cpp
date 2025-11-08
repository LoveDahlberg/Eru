
#include "testParser.h"

TEST(Parser, TestSemanticExpressionFailure) {

  std::vector<failureTestCase> testCases;

  // Boolean not supported
  testCases.push_back({R"(
    int main(){
      bool one = true 
    } 
  )"});

  // Expression wrong type int and string
  testCases.push_back({R"(
    int main(){
      int one = 1 + "string"
    } 
  )"});

  // Expression wrong type int and string
  testCases.push_back({R"(
    int main(){
      int one = 1 - "string"
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

TEST(Parser, TestSemanticExpressionSuccess) {
  std::vector<std::string> testCases;

  // Integer in if with + and - for int literal, identifier and function call.
  testCases.push_back({R"(
    int something()
    
    int main(){
      int one = 4
      if(1 + 2 - 3 + one - something() - 1)
      {
        return 2
      }
      return 1
    } 
  )"}); 

  // String in return with string literal, identifier and function call  
  testCases.push_back({R"(
    string something()
    
    string main(){
      string one = "a"
      return "1" + "2" - "3" or "one" and something() - "1"
    } 
  )"}); 

  // Recursive function calls as parameters
  testCases.push_back({R"(
    int something(int a, int b, int c) {
      return 1
    }
    
    int main(){
      return something(something(something(something(1,2,3),something(4,something(5,6,something(7,8,9)),10),11),12,13),14,15)
    } 
  )"}); 

  for (auto testCase : testCases) {
    auto item = RunParser(testCase);
    ASSERT_TRUE(item.success);
  }
}