# Find locally installed Google Test
find_package(GTest REQUIRED)              # Core Google Test
find_package(Threads REQUIRED)            # Required for threading (GTest dependency)

enable_testing()

# Include lib folder to enable including cpp files in unit tests.
include_directories(${CMAKE_SOURCE_DIR}/lib)

add_subdirectory(Lexer)
add_subdirectory(Support)

# Add a custom target for running all unit tests
add_custom_target(unittests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS testLexer testFileReader
    COMMENT "Running all unit tests"
)