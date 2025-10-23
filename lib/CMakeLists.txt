# Create a library target for Lexer
add_library(Lexer STATIC
    Lexer/Lexer.cpp)
target_include_directories(Lexer PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(Lexer PRIVATE ${llvm_libs})

# Create a library target for Support
add_library(Support STATIC
    Support/IO/FileReader.cpp
)
target_include_directories(Support PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(Support PRIVATE ${llvm_libs})

# Create a library target for IR
add_library(IR STATIC
    IR/Assignment.cpp
    IR/Controlflow.cpp
    IR/Expression.cpp
    IR/Function.cpp
    IR/VariableDeclaration.cpp
    IR/IRGenerator.cpp
)
target_include_directories(IR PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(IR PRIVATE ${llvm_libs} Support)

# Create a library target for Analyzer
add_library(Analyzer STATIC
    Analyzer/Function.cpp
    Analyzer/Variable.cpp
    Analyzer/Expression.cpp
    Analyzer/Analyzer.cpp
)
target_include_directories(Analyzer PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(Analyzer PRIVATE ${llvm_libs} Support)

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
target_link_libraries(Parser PRIVATE ${llvm_libs} Lexer Analyzer)

# Create a library target for Frontend
add_library(Frontend STATIC
    Frontend/Actions.cpp
    Frontend/Compiler.cpp
    Frontend/Linker.cpp
    Frontend/EmitObjectFile.cpp
)
target_include_directories(Frontend PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(Frontend PRIVATE ${llvm_libs} Lexer Parser IR)