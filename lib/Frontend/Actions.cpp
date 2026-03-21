#include <Frontend/Actions.h>

namespace Frontend::Action {

CompilationAction *GetAction(const std::string &relevantArgument) {
  if (!argumentToActionKind.contains(relevantArgument)) {
    return nullptr;
  }

  return GetAction(argumentToActionKind.at(relevantArgument));
}

CompilationAction *GetAction(const ActionKind &actionKind) {

  // switch (actionKind) {
  // case EmitObj:
  //   return new EmitObjectFile();
  // default:
  //   return nullptr;
  // }
  return nullptr;
}

} // namespace Frontend::Action