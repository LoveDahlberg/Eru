
#include <AST/AST.h>
#include <AST/Types.h>

namespace AST::Function {

class Function : public AST {
public:
  Function() {}

  llvm::Value *codegen();

private:
};

} // namespace AST::Function