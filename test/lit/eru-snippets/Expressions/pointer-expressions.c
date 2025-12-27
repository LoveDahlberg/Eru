// Test that pointer dereference and reference is correctly handled in expressions.

// RUN: rm -rf %t
// RUN: %split-file %s %t

// Compile main.arda
// RUN: %processor %t/main.arda -o %t/program

// Run the program and test its output
// RUN: %print-exit-code %t/program 1 | FileCheck %s

// CHECK: Exit Code: 8 

//--- main.arda


int Valinor(int argc) [] {
  int result = 0

  // 1. Modify a variable that is stored as a value -> for example a parameter
  argc = 1
  result = result + argc

  // 2. Modify a value that is stored as a stack pointer.
  int one
  one = 1
  result = result + one
  
  // 3. Get address of variable that is stored as a value
  int& two = &argc
  result = result + *two

  // 4. Get address of a variable that is stored as a stack pointer. 
  int three = 1
  int& four = &three
  result = result + *four

  // 5. Indirection level 2.
  int five = 1
  int &six = &five
  int &&seven = &six
  result = result + **seven

  // 6. Indirection level 5.
  int eight = 1
  int &nine = &eight
  int &&ten = &nine
  int &&&eleven = &ten
  int &&&&twelve = &eleven
  int &&&&&thirteen = &twelve
  result = result + *****thirteen

  // 7. Indirection up and down
  int fourteen = 1
  int &fifteen = &fourteen
  int &&sixteen = &fifteen
  int &&&seventeen = &sixteen
  int &&eighteen = *seventeen
  int &nineteen = *eighteen
  int twenty = *nineteen
  fifteen = **seventeen
  fourteen = ***seventeen + 1
  result = result + fourteen - 1

  // 8. Multi-level single line indirection
  int twentyone = 1
  int &twentytwo = &twentyone
  int &&twentythree = &twentytwo
  int &&&twentyfour = &twentythree
  int &bigBoy = &*&**&*&**twentyfour
  result = result + *bigBoy
  
  return result
}
