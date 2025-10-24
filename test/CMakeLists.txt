add_subdirectory(unit)
add_subdirectory(lit)

add_custom_target(test
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target unittests
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target lit
    COMMENT "Running all tests"
)