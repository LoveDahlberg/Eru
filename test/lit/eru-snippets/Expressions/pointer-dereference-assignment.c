
// Test that assignment to pointer dereference values works.

// RUN: rm -rf %t
// RUN: %split-file %s %t

// Compile main.arda
// RUN: %processor %t/main.arda -o %t/program -emit-llvm

// Run the program and test its output
// RUN: %print-exit-code %t/program | FileCheck %s

// CHECK: Exit Code: 3

//--- main.arda

int Valinor()[] {
  int a = 0
  int& b = &a
  int&& c = &b
  int&&& d = &c
  **c = 1
  ***d = *b + 1
  *b = ***d + **c - 1
  return a
}