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
    IR/VariableDeclaration.cpp
    IR/CompilationUnit.cpp
    IR/Types.cpp
    IR/Function.cpp
    IR/Statement.cpp
    IR/Assignment.cpp
    IR/Controlflow.cpp
    IR/Expression.cpp
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
)
target_include_directories(Parser PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(Parser PRIVATE ${llvm_libs} Lexer IR)

# Create a library target for Analyzer
add_library(Analyzer STATIC
    Analyzer/Analyzer.cpp
)
target_include_directories(Analyzer PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(Analyzer PRIVATE ${llvm_libs})

# Create a library target for Frontend
add_library(Frontend STATIC
    Frontend/Actions.cpp
    Frontend/Compiler.cpp
)
target_include_directories(Frontend PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(Frontend PRIVATE ${llvm_libs} Lexer Parser AST)
