// Test that functions can return any level of indirection pointers.

// RUN: rm -rf %t
// RUN: %split-file %s %t

// Compile main.arda
// RUN: %processor %t/main.arda -o %t/program -emit-llvm

// Run the program and test its output
// RUN: %print-exit-code %t/program | FileCheck %s

// CHECK: Exit Code: 3

//--- main.arda

int firstData = 0
int& first() {
  firstData = 1
  return &firstData
}

int second(int& a) {
  return *a
}

int&& third(int&&&& b) {
  return **b
}

int Valinor() [] {
  int& data = first()
  int data2 = second(data)

  int&& data3 = &data
  int&&& data4 = &data3
  int&& data5 = third(&data4)

  return *data + data2 + **data5
}