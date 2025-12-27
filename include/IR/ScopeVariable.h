#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/NoFolder.h>

namespace IR {
struct ScopeVariable {
  ScopeVariable()
      : variable(nullptr), underlyingType(nullptr), isAllocaValue(false),
        pointerIndirectionCount(0) {}

  /// Creates a pointer scope variable.
  ScopeVariable(llvm::Value *variable, llvm::Type *type, bool isAllocaValue,
                int pointerIndirectionCount)
      : variable(variable), underlyingType(type), isAllocaValue(isAllocaValue),
        pointerIndirectionCount(pointerIndirectionCount) {}

  llvm::Value *variable;
  llvm::Type *underlyingType;

  /// Get the value of the stored variable.
  llvm::Value *getValue(llvm::IRBuilder<llvm::NoFolder> *builder);

  /// Dereference the variable to be used in an assignment.
  llvm::Value *dereferenceAssignment(llvm::IRBuilder<llvm::NoFolder> *builder,
                                     int indirectionStepsToTake);

  /// Dereference the variable to be used in an expression.
  llvm::Value *dereferenceExpression(llvm::IRBuilder<llvm::NoFolder> *builder,
                                     int indirectionStepsToTake);

  /// Get address of the variable.
  llvm::Value *getAddress(llvm::IRBuilder<llvm::NoFolder> *builder);

private:
  /// Dereference the variable down indirectionStepsToTake levels down.
  llvm::Value *dereference(llvm::IRBuilder<llvm::NoFolder> *builder,
                           int indirectionStepsToTake);
  bool isAllocaValue;
  int pointerIndirectionCount;
};

} // namespace IR