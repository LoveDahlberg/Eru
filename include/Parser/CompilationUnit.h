// stl
#include <optional>

#include <Parser/Parser.h>

// #include

namespace Parser {

std::optional<ParserItems> ParseCompilationUnit(Lexer &lexer);
}