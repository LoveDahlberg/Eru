add_custom_target(lit
    COMMAND ${CMAKE_COMMAND} -E env BUILD=${CMAKE_BINARY_DIR}
    lit -v ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS eru
    COMMENT "Running all lit tests"
)