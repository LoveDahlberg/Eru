#include <Frontend/Actions.h>

namespace Frontend::Action {

Action *GetAction(const std::string &relevantArgument) {
  if (!argumentToActionKind.contains(relevantArgument)) {
    return nullptr;
  }

  return GetAction(argumentToActionKind.at(relevantArgument));
}

Action *GetAction(const ActionKind &actionKind) {

  // switch (actionKind) {
  // case EmitObj:
  //   return new EmitObjectFile();
  // default:
  //   return nullptr;
  // }
}

} // namespace Frontend::Action