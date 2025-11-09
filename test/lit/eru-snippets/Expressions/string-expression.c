// Test string expressions.

// XFAIL: *

// RUN: rm -rf %t
// RUN: %split-file %s %t

// Compile main.arda
// RUN: %processor %t/main.arda -o %t/program -c

//--- main.arda

string something() []{
  return "1"
}

int Valinor(int arg){
  string one = "a"
  string two = "1" + "2" - "3" or "one" and something() - "1"
  return 1
}