// stl
#include <optional>

#include <Parser/Syntax/Syntax.h>

// #include

namespace Parser::Syntax {

std::optional<syntaxItems> ParseCompilationUnit(Lexer &lexer);
}