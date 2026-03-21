#pragma once

#include <Frontend/Actions.h>

#include <AST/ASTContext.h>
#include <Support/IO/File.h>

namespace Frontend::Action {

class BinaryRewriter : public BinaryAction {
public:
  virtual Error ActOn(const Support::IO::Files& files) override;
};

} // namespace Frontend::Action

namespace Frontend {

Error Rewrite(Action::BinaryAction *action, const Support::IO::Files &files);

} // namespace Frontend
