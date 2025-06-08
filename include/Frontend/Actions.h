#pragma once

#include <string>
#include <unordered_map>

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
  virtual bool ActOn() = 0;
};

class EmitObjectiveFile : public Action {
  virtual bool ActOn() override;
};

Action *GetAction(std::string relevantArgument);

} // namespace Frontend::Action