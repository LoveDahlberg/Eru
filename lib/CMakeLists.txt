# Create a library target for Lexer
add_library(Lexer STATIC
    Lexer/Lexer.cpp
    Lexer/Tokens.cpp)
target_include_directories(Lexer PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Create a library target for Support
add_library(Support STATIC
    Support/IO/File.cpp
    Support/Triple.cpp
    Support/Commandline.cpp
)
target_include_directories(Support PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Create a library target for IR
add_library(IR STATIC
    IR/Assignment.cpp
    IR/Controlflow.cpp
    IR/Expression.cpp
    IR/Function.cpp
    IR/VariableDeclaration.cpp
    IR/IRGenerator.cpp
    IR/ScopeVariable.cpp
)
target_include_directories(IR PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(IR PRIVATE Support)

# Create a library target for Analyzer
add_library(Analyzer STATIC
    Analyzer/Function.cpp
    Analyzer/Variable.cpp
    Analyzer/Expression.cpp
    Analyzer/Analyzer.cpp
)
target_include_directories(Analyzer PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(Analyzer PRIVATE Support)

# Create a library target for Parser
add_library(Parser STATIC
    Parser/VariableDeclaration.cpp
    Parser/Assignment.cpp
    Parser/CompilationUnit.cpp
    Parser/ControlFlow.cpp
    Parser/Directive.cpp
    Parser/Expression.cpp
    Parser/Function.cpp
    Parser/Identifier.cpp
    Parser/Literal.cpp
    Parser/Statement.cpp
    Parser/Type.cpp
    Parser/Parser.cpp
)
target_include_directories(Parser PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(Parser PRIVATE Lexer Analyzer)

# Create a library target for Rewriter
add_library(LibRewriter STATIC
    Rewriter/Discover/Discover.cpp
    Rewriter/Discover/Relocations.cpp
    Rewriter/Discover/Sections.cpp
    Rewriter/Discover/Symbols.cpp
    Rewriter/Discover/VirtualAddress.cpp
    Rewriter/Disassemble.cpp
    Rewriter/Emit.cpp
    Rewriter/ExamplePass.cpp
    Rewriter/ObjectRewriter.cpp
)
target_include_directories(LibRewriter PUBLIC ${CMAKE_SOURCE_DIR}/include ${BOLT_INCLUDE_DIRS})
target_link_libraries(LibRewriter PRIVATE Support)

# Create a library target for Frontend
add_library(Frontend STATIC
    Frontend/Actions.cpp
    Frontend/Compiler.cpp
    Frontend/Linker.cpp
    Frontend/EmitObjectFile.cpp
    Frontend/Rewriter.cpp
)

target_include_directories(Frontend PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(Frontend PRIVATE Lexer Parser IR Support)
