
// Test that the scope of variables can be accessed correctly and that
// shadowing of variables works.

// RUN: %split-file %s %t

// Compile test.arda
// RUN: %processor %t/test.arda -o %t/test.o -c

// Compile support program
// RUN: clang %p/Input/main.c -o %t/main.o -c

// Link C program with eru object
// RUN: clang %t/main.o %t/test.o -o %t/program

// Run the program and test its output
// RUN: %t/program 7 | FileCheck %s

// CHECK: Exit Code: 20

//--- test.arda

int d = 0

int Valinor(int a) [] {
  int g = 3 + a

  if(1) {
    d = g + 1
    int i = 1
    if(1) {
      int g = d + a
      d = g + 1 + i
    }
  }
  return d
}