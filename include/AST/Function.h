
#include <AST/AST.h>
#include <AST/Types.h>
#include <AST/Declaration.h>

namespace AST::Function {

class Function {
public:
  Function(Declaration::FunctionDeclaration* declaration) {}

  llvm::Value *codegen();

private:
};

// Add pirmary expression and subclasses..

} // namespace AST::Function