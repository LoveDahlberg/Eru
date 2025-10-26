add_subdirectory(startup)

# Each subdirectory should add to the STDLIB_OBJECTS list.
add_library(Ainur STATIC ${STDLIB_OBJECTS})

# Place Ainur in its own dedicated dir.
set_target_properties(Ainur PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Ainur")

target_link_options(Ainur PRIVATE -nostdlib)