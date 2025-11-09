// Test function call expression.

// RUN: rm -rf %t
// RUN: %split-file %s %t

// Compile main.arda
// RUN: %processor %t/main.arda -o %t/program

// RUN: %print-exit-code %t/program | FileCheck %s

// CHECK: Exit Code: 120

//--- main.arda

int something(int a, int b, int c) {
  return a + b + c
}

int Valinor(){
  return something(something(something(something(1,2,3),something(4,something(5,6,something(7,8,9)),10),11),12,13),14,15)
}

// r = a + 14 + 15
// a = b + 12 + 13 = 91
// b =  1 + 2 + 3 + C + 11 = 66 
// c =  4 + d + 10 = 49
// d = 5 + 6 + 7 + 8 + 9 = 35
