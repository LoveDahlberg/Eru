cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wunused-result)

# LLVM is complied with this, so we must also use it.
add_compile_options(-fno-rtti)

# Use libstdc++ as project only supports Linux.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libstdc++ -gdwarf-4")

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

set(LLVM_PATH "" CACHE PATH "Path to LLVM+BOLT install prefix")
if(NOT LLVM_PATH)
  message(FATAL_ERROR "LLVM_PATH not set. Pass -DLLVM_PATH=/path/to/llvm-install")
endif()

find_package(LLVM REQUIRED CONFIG PATHS ${LLVM_PATH}/lib/cmake/llvm NO_DEFAULT_PATH)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")

include_directories(${LLVM_PATH}/include)
link_directories(${LLVM_PATH}/lib)
add_definitions(${LLVM_DEFINITIONS})

# Find all llvm libs
set(llvm_libs ${LLVM_AVAILABLE_LIBS})

############################
# Includes and Libraries
############################

# Include the common headers
include_directories(${CMAKE_SOURCE_DIR}/include)

set(CMAKE_BUILD_TYPE Debug)

add_subdirectory(test)
add_subdirectory(lib)
add_subdirectory(tools)

############################
# Create Eru target
############################

# Add each lib folder to each own lib that can be included here.
add_executable(eru lib/Driver/Eru.cpp)

# Place in bin folder
set_target_properties(eru PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Link dependencies
target_link_libraries(eru PRIVATE
    # LLVM dependecies
    ${llvm_libs}
    LibEdit::LibEdit
    zstd::libzstd_shared
    CURL::libcurl
    
    # Project dependencies
    Frontend
    Support)

############################
# Create Rewriter target 
############################

add_executable(rewriter lib/Driver/Rewriter.cpp)

set_target_properties(rewriter PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_link_libraries(rewriter PRIVATE
    # Project dependencies    
    Support
    Frontend
    LibRewriter

    # LLVM dependecies
    -Wl,--start-group
    ${llvm_libs}
    -Wl,--end-group
    LibEdit::LibEdit
    zstd::libzstd_shared
    CURL::libcurl
)