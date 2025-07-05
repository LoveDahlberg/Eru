

#include "AST/Types.h"
#include <AST/Function.h>
#include <Support/Result.h>

namespace Analyzer {

class PrivateAnalyzer;

class FunctionAnalyzer {

  PrivateAnalyzer &analyzer;

  std::vector<AST::Function::Function *> functions;

  /// Attempt to add a function. Returns an error if not successful.
  Result<bool> addFunction(AST::Function::Function *function,
                           AST::Function::FunctionStatus status);

public:
  FunctionAnalyzer(PrivateAnalyzer &analyser) : analyzer(analyser) {}

  Result<bool> ActOnDeclaration(AST::Function::Function *function);
  Result<bool> ActOnDefinition(AST::Function::Function *function);
  Result<bool>
  ActOnCall(AST::Function::FunctionCall *call,
            AST::Types::Types expectedReturnValue = AST::Types::NONE);
};

} // namespace Analyzer