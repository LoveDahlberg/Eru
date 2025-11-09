// Test that the same function can be declared multiple times.

// RUN: rm -rf %t
// RUN: %split-file %s %t

// Compile main.arda
// RUN: %processor %t/main.arda -o %t/program -c 

//--- main.arda

int something(int a)
int something(int a)