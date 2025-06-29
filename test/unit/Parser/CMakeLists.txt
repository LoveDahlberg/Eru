
include_directories(${CMAKE_CURRENT_LIST_DIR})

# Create the test executable
add_executable(testParser testParser.cpp testFunctionAnalysis.cpp)

# Link against local GTest installation
target_link_libraries(testParser PRIVATE 
    # ${LLVM_LIBRARIES}
    GTest::GTest 
    GTest::Main
    Threads::Threads
    # Add linking agains the specific lib.
    Parser
    IR
)

# Register the test with CTest
add_test(NAME Parser COMMAND testParser)