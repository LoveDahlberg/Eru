cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wunused-result)

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
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}, ${LLVM_INCLUDE_DIRS}")
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(llvm_libs -20)
add_compile_definitions(_LIBCPP_DISABLE_DEPRECATION_WARNINGS)
add_compile_options(-glldb)

############################
# BOLT Setup
############################

set(BOLT_SOURCE_DIR "" CACHE PATH "Path to llvm-project root from sparse checkout")

if(NOT BOLT_SOURCE_DIR)
  message(FATAL_ERROR "BOLT_BUILD_DIR not set. Pass -DBOLT_BUILD_DIR=/path/to/bolt/build")
endif()

set(BOLT_INCLUDE_DIRS
    ${BOLT_SOURCE_DIR}/bolt/include   # headers from source
)
# link_directories(${BOLT_BUILD_DIR}/lib)
link_directories(/usr/lib/llvm-20/lib)

set(BOLT_LIBS
    LLVMBOLTCore
    LLVMBOLTPasses
    LLVMBOLTProfile
    LLVMBOLTRewrite
    LLVMBOLTRuntimeLibs
    LLVMBOLTTargetX86
    LLVMBOLTTargetAArch64
    LLVMBOLTTargetRISCV
    LLVMBOLTUtils
)

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

set(LLVM_TARGET_LIBS
    LLVMX86CodeGen LLVMX86Desc LLVMX86Info LLVMX86AsmParser LLVMX86Disassembler
    LLVMAArch64CodeGen LLVMAArch64Desc LLVMAArch64Info LLVMAArch64AsmParser LLVMAArch64Disassembler
    LLVMRISCVCodeGen LLVMRISCVDesc LLVMRISCVInfo LLVMRISCVAsmParser LLVMRISCVDisassembler
)

target_link_libraries(rewriter PRIVATE
    -Wl,--start-group
    ${llvm_libs}
    ${LLVM_TARGET_LIBS}
    ${BOLT_LIBS}
    Support
    Frontend
    -Wl,--end-group

    # LLVM dependecies
    LibEdit::LibEdit
    zstd::libzstd_shared
    CURL::libcurl
)