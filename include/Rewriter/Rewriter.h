
//#undef ERU_SUCCESS
#include <bolt/Core/BinaryContext.h>
#include <bolt/Passes/BinaryPasses.h>
//#define ERU_SUCCESS Result<>()

namespace Rewriter {

class MyRewritePass : public llvm::bolt::BinaryFunctionPass {
public:
  MyRewritePass() : BinaryFunctionPass(/*PrintPass=*/false) {}

  const char *getName() const override { return "my-rewrite-pass"; }

  llvm::Error runOnFunctions(llvm::bolt::BinaryContext &BC) override;
};

} // namespace Rewriter