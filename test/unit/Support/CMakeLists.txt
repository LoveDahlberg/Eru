# Create the test executable
add_executable(testFile IO/testFile.cpp)

# Link against local GTest installation
target_link_libraries(testFile PRIVATE 
    GTest::GTest 
    GTest::Main
    Threads::Threads
    
    # Add linking agains the specific lib.
    Support

    ${llvm_libs}
)
# target_include_directories(testFile PRIVATE ${CMAKE_SOURCE_DIR}/lib)

# Register the test with CTest
add_test(NAME FileReader COMMAND testFile)