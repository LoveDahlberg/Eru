
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include <Lexer/Lexer.h>
#include <Lexer/Tokens.h>

#include <AST/CompilationUnit.h>
#include <AST/Types.h>

using namespace Lexing;
using namespace AST;

namespace Parser {

struct parserItems {
  parserItems(Lexer &lexer)
      : context(new llvm::LLVMContext()), module(new llvm::Module("", *context)),
        lexer(lexer) {}
private:
  llvm::LLVMContext* context;
public:
  llvm::Module* module;
  Lexer lexer;
  CompilationUnit compilationUnit;
};

std::optional<std::string> ParseIdentifier(parserItems &items);

std::optional<llvm::Type *> ParseType(parserItems &items) ;

std::optional<std::vector<Declaration::VariableDeclaration*>>
ParseParameters(parserItems &items) ;

bool ParseFunctionDefinition(
  parserItems &items, Declaration::FunctionDeclaration* declaration);

bool ParseFunctionDefinitionOrDeclaration(parserItems &items, llvm::Type *type,
    std::string &identifier);

bool ParseDeclarationOrFunction(parserItems &items);

parserItems ParseCompilationUnit(Lexer &lexer);

} // namespace Parser