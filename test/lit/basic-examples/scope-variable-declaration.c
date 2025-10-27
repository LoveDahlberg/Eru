// Test that the scope of variables can be accessed correctly and that
// shadowing of variables works.

// RUN: rm -rf %t
// RUN: %split-file %s %t

// Compile test.arda
// RUN: %processor %t/main.arda -o %t/program

// Run the program and test its output
// RUN: %print-exit-code %t/program 1 | FileCheck %s

// CHECK: Exit Code: 10

//--- main.arda

int d = 0

int Valinor(int a) [] {
  // 5 = 3 + 2
  int g = 3 + a

  if(1) {
    // 6 = 5 + 1
    d = g + 1
    int i = 1
    if(1) {
      // 8 = 2 + 6
      int g = d + a
      // 10 = 8 + 1 + 1
      d = g + 1 + i
    }
  }
  // 10
  return d
}