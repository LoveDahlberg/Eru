
add_library(startup_obj OBJECT startup.c)

# Clear compiler options from main project, these differ from the ones 
# set here (we build without C stdlib which makes certain flags not work
# for example).

set_target_properties(startup_obj PROPERTIES
    COMPILE_OPTIONS ""
    COMPILE_DEFINITIONS ""
    INCLUDE_DIRECTORIES ""
)

target_compile_options(startup_obj PRIVATE -nostdlib)

# Set the STDLIB_OBJECTS defined in parent CMakeLists.txt to add this object to the Ainur archive.
set(STDLIB_OBJECTS ${STDLIB_OBJECTS} $<TARGET_OBJECTS:startup_obj> PARENT_SCOPE)