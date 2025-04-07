cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")

project(project)

# Generate a compile_commands.json file. Used for the clangd extension.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

############################
# LLVM Setup
############################

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

llvm_map_components_to_libnames(llvm_libs -19)
add_compile_definitions(_LIBCPP_DISABLE_DEPRECATION_WARNINGS)
add_compile_options(-glldb)

############################
# Includes and Libraries
############################

# Include the common headers
include_directories(${CMAKE_SOURCE_DIR}/include)

set(CMAKE_BUILD_TYPE Debug)

add_subdirectory(test)
add_subdirectory(lib)

############################
# Create main target
############################

# Add each lib folder to each own lib that can be included here.
add_executable(main lib/Main.cpp)

# Link dependencies
target_link_libraries(main PRIVATE 
    # LLVM dependecies
    ${llvm_libs}
    LibEdit::LibEdit
    zstd::libzstd_shared
    CURL::libcurl
    
    # Project dependencies
    Lexer
    FileReader)

add_custom_target(run
    COMMAND $<TARGET_FILE:main>
    DEPENDS main
    COMMENT "Compile and run my_program"
  )