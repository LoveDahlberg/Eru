# The Eru programming language

## What is it

## Example program

## Goals

## How to build

The project depends on the LLVM project with bolt. Build this from source

### Build llvm with bolt

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

Apply custom patch (TODO)

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
Lastly, copy bolt headers into the install directory

```
cp -r llvm-project/bolt/include/bolt <llvm-install-dir>/include/
```

### Build Project

Then clone and build the project

```
git clone https://github.com/LoveDahlberg/Eru.git
mkdir build
cd build
env CXX=clang++ cmake ../Eru -DLLVM_DIR=<llvm-install-dir>
make all
```