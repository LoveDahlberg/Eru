#pragma once

#include <Frontend/Actions.h>

#include <AST/ASTContext.h>
#include <Support/IO/File.h>

namespace Frontend::Action {

class ObjectRewriter : public RewriteAction {
public:
  virtual Error ActOn(const Support::IO::Files& files) override;
};

} // namespace Frontend::Action

namespace Frontend {

Error RewriteObject(Action::RewriteAction *action, const Support::IO::Files &files);

} // namespace Frontend
