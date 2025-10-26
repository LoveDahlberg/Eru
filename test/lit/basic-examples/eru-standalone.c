// Test that a simple ERU program can run stand alone.

// RUN: %split-file %s %t

// Compile main.arda
// RUN: %processor %t/main.arda -o %t/program

// RUN: %print-exit-code %t/program one two | FileCheck %s

// CHECK: Exit Code: 45

//--- main.arda

int Valinor(int argc) [] {
  return argc + 42
}
