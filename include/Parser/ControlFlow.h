#pragma once

#include <Parser/Parser.h>

#include <AST/Controlflow.h>

using namespace ::AST::Controlflow;

namespace Parser::Controlflow {
std::optional<ConditionalBranchingGroup *>
ParseConditionalBranchingGroup(ParserItems &items);

}