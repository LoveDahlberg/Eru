// llvm
#include <llvm/Object/ELFObjectFile.h>

// bolt
#include <bolt/Core/BinaryContext.h>

#include <Support/Result.h>

namespace Rewriter {

Error DiscoverAndRegisterObject(llvm::object::ELF64LEObjectFile *elfFile,
                                llvm::bolt::BinaryContext *binaryContext);

}