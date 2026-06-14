# The Eru programming language

## What is it

## Example program

## Goals

## How to build

The project depends on the LLVM project with bolt. Build this from source.


### Build Project

Clone project

```
cd project
git clone https://github.com/LoveDahlberg/Eru.git
```

#### Build llvm with bolt

First clone llvm. The project currently built for LLVM 22.1.1.

```
git clone \
  --depth 1 \
  --filter=blob:none \
  --sparse \
  --branch llvmorg-22.1.1 \
  https://github.com/llvm/llvm-project

git sparse-checkout set llvm bolt cmake third-party 
git checkout
```

Apply custom patch
```
git apply project/vendor/llvm/binaryFuncition_public_symbols.patch
```

Then configure and install llvm with bolt

```
cmake -G Ninja ../llvm-project/llvm \
  -DCMAKE_INSTALL_PREFIX=<llvm-install-dir> \
  -DLLVM_TARGETS_TO_BUILD="X86" \
  -DCMAKE_BUILD_TYPE=Release \
  -DLLVM_ENABLE_ASSERTIONS=OFF \
  -DLLVM_ENABLE_PROJECTS="bolt" \
  -DLLVM_INCLUDE_TESTS=OFF \
  -DLLVM_INCLUDE_BENCHMARKS=OFF \
  -DBOLT_INCLUDE_TESTS=OFF

ninja install
```

or if building minimally with Debug enabled
```
cmake -G Ninja ../llvm-project/llvm \
  -DCMAKE_INSTALL_PREFIX=<llvm-install-dir> \
  -DLLVM_TARGETS_TO_BUILD="X86" \
  -DCMAKE_BUILD_TYPE=Debug \
  -DLLVM_ENABLE_ASSERTIONS=ON \
  -DLLVM_ENABLE_DUMP=ON \
  -DLLVM_ENABLE_PROJECTS="bolt" \
  -DLLVM_INCLUDE_TESTS=OFF \
  -DLLVM_INCLUDE_BENCHMARKS=OFF \
  -DBOLT_INCLUDE_TESTS=OFF \
  -DLLVM_BUILD_TOOLS=OFF \
  -DLLVM_USE_LINKER=lld \
  -DLLVM_PARALLEL_LINK_JOBS=1 \
  -DLLVM_USE_SPLIT_DWARF=ON

ninja install
```

Lastly, copy bolt headers into the install directory

```
cp -r llvm-project/bolt/include/bolt <llvm-install-dir>/include/
```

#### Build project 

Build the cloned project using the built LLVM

```
cd project
mkdir build
cd build
env CXX=clang++ cmake ../Eru -DLLVM_DIR=<llvm-install-dir>
make all
```