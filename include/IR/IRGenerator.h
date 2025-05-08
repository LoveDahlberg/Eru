#include <AST/CompilationUnit.h>

namespace IR {

std::vector<llvm::Value *> GenerateIR(AST::CompilationUnit compilationUnit,
                                      llvm::Module &module);

}