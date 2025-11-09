// Test boolean expressions

// XFAIL: *

// RUN: rm -rf %t
// RUN: %split-file %s %t

// Compile main.arda
// RUN: %processor %t/main.arda -o %t/program

//--- main.arda

int Valinor(int argc) [] {
  bool a = true
  return 0
}
