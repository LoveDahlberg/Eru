
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

# Create a library target for FileReader
add_library(Parser STATIC
    Parser/Parser.cpp
)
target_include_directories(Parser PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(Parser PRIVATE ${llvm_libs} Lexer)