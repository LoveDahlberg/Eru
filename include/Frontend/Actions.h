#pragma once

// llvm
#include <llvm/IR/Module.h>

#include <string>
#include <unordered_map>

#include <Support/Result.h>
#include <AST/ASTContext.h>

// - GetAction defines what actions we can have and it returns which one we
// should use here.
//  - We could have:
//   - EmitObj: normal mode, creates obj files that we then link together.
//   - ASTDump/ASTPrint: parses AST and prints/dumps it
//   - EmitLLVM. emits ll file.
//   - EmitBC: emits a bc file.
//   - ParseOnly: parses and applies analyzis and outputs any error/warnings.
//   For testing.
//   // Could have one that doesn't perform IR passes.
//  - Those that generates IR and applies passes on the AST are CodeGenActions,
//  they have some common code.
//   - The things that act on the AST could be the consumers (?) But this
//   abstraction might be overkill.

namespace Frontend::Action {

enum ActionKind { EmitObj };

static const std::unordered_map<std::string, ActionKind> argumentToActionKind =
    {{"obj", ActionKind::EmitObj}};

class Action {
public:
  virtual Error ActOn(AST::Context::ASTContext) = 0;
};

class EmitObjectFile : public Action {
public:  
  EmitObjectFile(const std::string &outputFile) : outputFile(outputFile) {}
  
  virtual Error ActOn(AST::Context::ASTContext) override;

private:

  bool emitObject(llvm::Module &module);

  const std::string &outputFile;
};

Action *GetAction(const std::string &relevantArgument);
Action *GetAction(const ActionKind &actionKind);

} // namespace Frontend::Action