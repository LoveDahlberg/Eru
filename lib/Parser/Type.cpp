
#include <Parser/Type.h>

namespace Parser::Type {

std::optional<llvm::Type *> ParseType(Parser &items) {

    llvm::Type *type;
  
    // TODO should avoid having to use the LLVM context here, as we don't
    // pass it on to the IR generator later. Should maybe just store the token here.
    switch (items.lexer.getCurrentToken().type) {
    case TokenType::INT:
      type = llvm::Type::getInt32Ty(items.module->getContext());
      break;
    case TokenType::SIGNED_INT_32:
      type = llvm::Type::getInt32Ty(items.module->getContext());
      break;
    case TokenType::UNSIGNED_INT_32:
      type = llvm::Type::getInt32Ty(items.module->getContext());
      break;
    case TokenType::BOOl:
      type = llvm::Type::getInt1Ty(items.module->getContext());
      break;
    case TokenType::CHAR:
      type = llvm::Type::getInt8Ty(items.module->getContext());
      break;
    case TokenType::STRING:
      // TODO implement string handling
      type = llvm::StructType::create(items.module->getContext(), "string");
      break;
    default:
      // err
      return std::nullopt;
    }
  
    // Get next, current type saved.
    items.lexer.generateNextToken();
    return type;
  }

}