// Tests that pointer arithmetic works

// XFAIL: *

// RUN: rm -rf %t
// RUN: %split-file %s %t

// Compile main.arda
// RUN: %processor %t/main.arda -o %t/program

// Run the program and test its output
// RUN: %print-exit-code %t/program | FileCheck %s

// CHECK: Exit Code: 2

//--- main.arda

int Valinor() [] {
  int a = 127
  int &b = &a + 1
  return *b
}