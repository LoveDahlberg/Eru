# Create a library target for Lexer
add_library(Lexer STATIC
    Lexer/Lexer.cpp)
target_include_directories(Lexer PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(Lexer PRIVATE ${llvm_libs})

# Create a library target for FileReader
add_library(FileReader STATIC
    Support/IO/FileReader.cpp
)
target_include_directories(FileReader PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(FileReader PRIVATE ${llvm_libs})

# Create a library target for AST
add_library(AST STATIC
    AST/Declaration.cpp
    AST/CompilationUnit.cpp
    AST/Types.cpp
    AST/Function.cpp
    AST/Statement.cpp
    AST/Assignment.cpp
    AST/Controlflow.cpp
    AST/Expression.cpp
)
target_include_directories(AST PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(AST PRIVATE ${llvm_libs})

# Create a library target for IR
add_library(IR STATIC
    IR/IRGenerator.cpp
)
target_include_directories(IR PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(IR PRIVATE ${llvm_libs} AST)

# Create a library target for Parser
add_library(Parser STATIC
    Parser/Syntax/VariableDeclaration.cpp
    Parser/Syntax/Assignment.cpp
    Parser/Syntax/CompilationUnit.cpp
    Parser/Syntax/ControlFlow.cpp
    Parser/Syntax/Directive.cpp
    Parser/Syntax/Expression.cpp
    Parser/Syntax/Function.cpp
    Parser/Syntax/Identifier.cpp
    Parser/Syntax/Literal.cpp
    Parser/Syntax/Statement.cpp
    Parser/Syntax/Type.cpp
)
target_include_directories(Parser PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(Parser PRIVATE ${llvm_libs} Lexer IR)
