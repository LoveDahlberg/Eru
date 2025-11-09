// Test expression in if statement.

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
  if(1 + 2 - 3 + one - something() - 1)
  {
    return 2
  }
  return 1
}