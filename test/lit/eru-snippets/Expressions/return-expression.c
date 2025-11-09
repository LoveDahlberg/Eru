// Test expression in return.

// RUN: rm -rf %t
// RUN: %split-file %s %t

// Compile main.arda
// RUN: %processor %t/main.arda -o %t/program

// RUN: %print-exit-code %t/program | FileCheck %s

// CHECK: Exit Code: 2

//--- main.arda

int something() [] {
  return 4
}
    
int Valinor(){
  int one = 4
  return 1 + 2 - 3 + one - something() + 2
}