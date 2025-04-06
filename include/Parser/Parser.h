
#include <Lexer/Lexer.h>
#include <Lexer/Tokens.h>

#include <AST/Top.h>
#include <AST/Types.h>

using namespace Lexing;
using namespace AST;

namespace Parser {

std::optional<Types::Identifier> ParseIdentifier(Lexer &lexer);

std::optional<Types::Type> ParseType(Lexer &lexer);

std::optional<std::vector<Declaration::Declaration>> ParseParameters(Lexer &lexer);

bool ParseFunctionDefinition(Lexer &lexer, Top &top, const Types::Type &type,
                             const Types::Identifier &identifier,
                             std::vector<Declaration::Declaration> parameters);

bool ParseFunctionDefinitionOrDeclaration(Lexer &lexer, Top &top, const Types::Type &type,
                                          const Types::Identifier &identifier);

bool ParseDeclarationOrFunction(Lexer &lexer, Top &top);

Top ParseTop(Lexer &lexer);

} // namespace Parser