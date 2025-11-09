// Test that function parameters can be shadowed.

// RUN: rm -rf %t
// RUN: %split-file %s %t

// Compile main.arda
// RUN: %processor %t/main.arda -o %t/program 

// RUN: %print-exit-code %t/program one | FileCheck %s

// CHECK: Exit Code: 3

//--- main.arda

int Valinor(int a) [] {
  if(1) {
    int a = 3
    return a
  }
  return 1
}