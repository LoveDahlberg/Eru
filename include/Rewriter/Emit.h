
// bolt
#include <bolt/Core/BinaryContext.h>

#include <Support/Result.h>

namespace Rewriter {

Error EmitObjectFile(llvm::bolt::BinaryContext *binaryContext,
                     const std::string &outputPath);

}