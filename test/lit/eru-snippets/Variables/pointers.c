// Test pointers

// RUN: rm -rf %t
// RUN: %split-file %s %t

// Compile main.arda
// RUN: %processor %t/main.arda -o %t/program

//--- main.arda

// Test that returning address to global works.
int global = 3
int& alsoSomething()
{
  return &global
}

int Valinor(int argc) [] {
  
  // Integer expressions are implicitly casted to addresses.
  int& something = 123 + 321
  something = 111 + 222

  // This also works, the pointer takes the value assigned 
  // to the variable.
  int aha = 1
  something = aha 
  int& ok = something

  // Get address of something and store as pointer
  something = &aha
  something = alsoSomething()

  // Dereference a pointer
  int ohno = *something

  // NULL is the same as address 0
  // something = NULL

  // Get address of a variable that holds a pointer
  int aa = 1
  int& bb = &aa 
  int&& cc = &bb

  // Dereference double pointer
  int dd = **cc

  // more levels.
  int secret = 42
  int &A = &secret
  int &&B = &A
  int &&&C = &B

  // What it should do (incase it breaks)
  // 5 of & and 8 of *. 8-5 = 3 dereferences.
  // This is the same number of levels available for C.
  int &result = &**&***&**&*&C

  int &&&&&& ohh = 123
  int & ah = *****ohh

  return 0
}