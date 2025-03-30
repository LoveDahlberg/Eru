cmake_minimum_required(VERSION 3.10)
project(project)

# Packages LLVM depends on.

# Libedit
find_path(LIBEDIT_INCLUDE_DIR NAMES histedit.h PATH_SUFFIXES editline)
find_library(LIBEDIT_LIBRARY NAMES edit)

if (LIBEDIT_INCLUDE_DIR AND LIBEDIT_LIBRARY)
  message(STATUS "Found LibEdit: ${LIBEDIT_LIBRARY}")
  include_directories(${LIBEDIT_INCLUDE_DIR})
else()
  message(FATAL_ERROR "LibEdit not found!")
endif()

# ZSTD
find_path(ZSTD_INCLUDE_DIR NAMES zstd.h)
find_library(ZSTD_LIBRARY NAMES zstd)

if (ZSTD_INCLUDE_DIR AND ZSTD_LIBRARY)
  message(STATUS "Found Zstandard: ${ZSTD_LIBRARY}")
  include_directories(${ZSTD_INCLUDE_DIR})
else()
  message(FATAL_ERROR "Zstandard (zstd) not found!")
endif()

find_package(CURL REQUIRED)

# Find LLVM and configure flags.
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}, ${LLVM_INCLUDE_DIRS}")
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Generate a compile_commands.json file. Used for the clangd extension.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include the common headers
include_directories(${CMAKE_SOURCE_DIR}/include)

add_subdirectory(test)

# Create target for main executable.
# Set the C++ standard to C++23
set(CMAKE_CXX_STANDARD 23)

# Add each lib folder to each own lib that can be included here.
add_executable(main lib/Main.cpp lib/Support/IO/FileReader.cpp)

llvm_map_components_to_libnames(llvm_libs core support irreader)
target_link_libraries(main PRIVATE 
    ${llvm_libs}
    LibEdit::LibEdit
    zstd::libzstd_shared
    CURL::libcurl)

add_custom_target(run
    COMMAND $<TARGET_FILE:main>
    DEPENDS main
    COMMENT "Compile and run my_program"
  )