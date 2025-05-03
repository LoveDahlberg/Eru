#pragma once

#include <Parser/Syntax/Syntax.h>

#include <AST/Controlflow.h>

using namespace ::AST::Controlflow;

namespace Parser::Syntax::Controlflow {
std::optional<ConditionalBranchingGroup *>
ParseConditionalBranchingGroup(syntaxItems &items);

}