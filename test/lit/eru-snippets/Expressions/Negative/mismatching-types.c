// Test that expressions with mixed types doesn't compile

// RUN: rm -rf %t
// RUN: %split-file %s %t

// Compile main.arda
// RUN: ! %processor %t/main.arda -o %t/program

//--- main.arda

int Valinor(int argc) [] {
  int one = 1 + "string" - 1
  return 0
}
