

#include <AST/Function.h>
#include <Support/Result.h>

namespace Analyzer {

class PrivateAnalyzer;

class FunctionAnalyzer {

  PrivateAnalyzer &analyzer;

  std::vector<AST::Function::Function *> functions;

  /// Attempt to add a function. Returns an error if not successful.
  Error addFunction(AST::Function::Function *function,
                    AST::Function::FunctionStatus status);

public:
  FunctionAnalyzer(PrivateAnalyzer &analyser) : analyzer(analyser) {}

  Error ActOnDeclaration(AST::Function::Function *function);
  Error ActOnDefinition(AST::Function::Function *function);

  /// Checks the call, its return type has to match with expectedReturnValue.
  Error ActOnCall(AST::Function::FunctionCall *call,
                  AST::Types::Types expectedReturnValue);

  /// Check the call, don't verify return value.
  Result<AST::Function::Function *>
  ActOnCall(AST::Function::FunctionCall *call);

  /// Declare the parameters of the given function in the current scope.
  Result<> ActOnParameters(AST::Function::Parameters parameters);
  
  AST::Function::Function *getFunction(std::string name);
};

} // namespace Analyzer