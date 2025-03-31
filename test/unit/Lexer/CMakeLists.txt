# Create the test executable
add_executable(testLexer testLexer.cpp)

# Link against local GTest installation
target_link_libraries(testLexer PRIVATE 
    GTest::GTest 
    GTest::Main
    Threads::Threads
    # Add linking agains the specific lib.
    Lexer
)

# Register the test with CTest
add_test(NAME Lexer COMMAND testLexer)