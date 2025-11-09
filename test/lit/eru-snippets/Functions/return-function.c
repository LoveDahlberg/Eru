// Test that inner function blocks can have a return or not.

// RUN: rm -rf %t
// RUN: %split-file %s %t

// Compile main.arda
// RUN: %processor %t/main.arda -o %t/program 

// RUN: %print-exit-code %t/program one | FileCheck %s

// CHECK: Exit Code: 1

//--- main.arda

int Valinor() [] {
  if(1) {
    int b = 1
    return b
  }
  elif(2) {
    return 2
  }
  elif(3) {
    // No return
  }
  return 4
}