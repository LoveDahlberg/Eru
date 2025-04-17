# Create the test executable
add_executable(testIR testIRGenerator.cpp)

# Link against local GTest installation
target_link_libraries(testIR PRIVATE
    GTest::GTest
    GTest::Main
    Threads::Threads

    # Add linking agains the specific lib.
    IR
    AST
)

# Register the test with CTest
add_test(NAME IR COMMAND testIR)