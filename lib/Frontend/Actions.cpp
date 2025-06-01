#include <Frontend/Actions.h>

namespace Frontend::Action {

Action *GetAction(std::string relevantArgument) {
  if (!argumentToActionKind.contains(relevantArgument)) {
    return nullptr;
  }

  switch (argumentToActionKind.at(relevantArgument)) {
  case EmitObj:
    return new EmitObjectiveFile();
  default:
    return nullptr;
  }
}

bool EmitObjectiveFile::ActOn() { return true; }

} // namespace Frontend::Action