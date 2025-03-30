# Find locally installed Google Test
find_package(GTest REQUIRED)              # Core Google Test
find_package(Threads REQUIRED)            # Required for threading (GTest dependency)

enable_testing()

# Create the test executable
add_executable(tests test.cpp)

# Link against local GTest installation
target_link_libraries(tests PRIVATE 
    GTest::GTest 
    GTest::Main
    Threads::Threads
    # Add linking agains the specific lib.
)

# Register the test with CTest
add_test(NAME TestCategory COMMAND tests)