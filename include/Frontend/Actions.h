#pragma once

// llvm
#include <llvm/IR/Module.h>

#include <filesystem>
#include <string>
#include <unordered_map>

#include <AST/ASTContext.h>
#include <Support/IO/File.h>
#include <Support/Result.h>

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

class CompilationAction {
public:
  virtual Error ActOn(AST::Context::ASTContext) = 0;
};

class BinaryAction {
public:
  virtual Error ActOn(const Support::IO::Files &files) = 0;
};

class EmitObjectFile : public CompilationAction {
public:
  EmitObjectFile(Support::IO::Files &files, const llvm::Triple &targetTriple,
                 bool emitLLVM)
      : files(files), targetTriple(targetTriple), emitLLVM(emitLLVM) {}

  virtual Error ActOn(AST::Context::ASTContext) override;

private:
  Support::IO::Files &files;
  const llvm::Triple &targetTriple;
  const bool emitLLVM;
};

enum ActionKind { EmitObj };

static const std::unordered_map<std::string, ActionKind> argumentToActionKind =
    {{"obj", ActionKind::EmitObj}};

CompilationAction *GetAction(const std::string &relevantArgument);
CompilationAction *GetAction(const ActionKind &actionKind);

} // namespace Frontend::Action