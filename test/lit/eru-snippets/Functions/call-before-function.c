// Test calling functions that are declared or defined after the call itself.

// RUN: rm -rf %t
// RUN: %split-file %s %t

// Compile main.arda
// RUN: %processor %t/main.arda %t/file.arda -o %t/program 

// RUN: %print-exit-code %t/program | FileCheck %s

// CHECK: Exit Code: 1

//--- main.arda

int Valinor(int a) [] {
  somethingElse(something(a))
  return something(somethingElse(a))
}

int something(int a)

int somethingElse(int a) [] {
  return 2
}

//--- file.arda

int something(int a) [] {
  return 1
}