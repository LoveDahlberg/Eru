# Create the test executable
add_executable(testFileReader IO/testFileReader.cpp)

# Link against local GTest installation
target_link_libraries(testFileReader PRIVATE 
    GTest::GTest 
    GTest::Main
    Threads::Threads
    
    # Add linking agains the specific lib.
    Support
)
# target_include_directories(testFileReader PRIVATE ${CMAKE_SOURCE_DIR}/lib)

# Register the test with CTest
add_test(NAME FileReader COMMAND testFileReader)