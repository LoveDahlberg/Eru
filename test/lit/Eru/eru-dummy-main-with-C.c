// Tests whether a simple object file can be created with Eru containing
// a C like main function and that that object file can link with a valid C program. 

// RUN: rm -rf %t
// RUN: %split-file %s %t

// Compile main.arda
// RUN: %processor %t/main.arda -o %t/main.o -c

// Compile test C program
// RUN: clang %t/test.c -o %t/test.o -c

// Link eru object file with c object file
// RUN: clang %t/main.o %t/test.o -o %t/program

// Run the program and test its output
// RUN: %print-exit-code %t/program one two | FileCheck %s

// CHECK: External 3
// CHECK: Exit Code: 8

//--- main.arda

int external(int input) []

int main(int numberOfArgs) [] {
  int response = external(numberOfArgs)

  return numberOfArgs + response
}

//--- test.c

#include <stdio.h>

int external(int input){
  printf("External %d\n", input);
  return 5;
}

