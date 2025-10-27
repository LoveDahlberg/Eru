// Test multiple input and output variations with and without -c, -o using arda
// and object files. 

// TODO: Verify content of .eru folder and content of error print.

// RUN: rm -rf %t
// RUN: %split-file %s %t

//
// -- Unkown type --
//

// Single unknown type
// RUN: ! %processor %t/main

// Multiple unknown type
// RUN: ! %processor %t/main %t/second

//
// -- Single input compile only --
//

// Single input compile-only no output
// RUN: %processor %t/main.arda -c
// RUN: test -e %t/main.o && rm -rf %t/main.o

// Single object input compile-only no output
// RUN: ! %processor %t/main.o -c

// Single input compile-only with output
// RUN: %processor %t/main.arda -o %t/program.o -c
// RUN: test -e %t/program.o && rm -rf %t/program.o

// Single input compile-only with output, same name
// RUN: ! %processor %t/main.arda -o %t/main.arda -c

// Single object input compile-only with output
// RUN: ! %processor %t/main.o -c -o %t/huh.o
// RUN: ! test -e %t/huh.o

//
// -- Multiple input compile only --
//

// Multiple input compile-only with output
// RUN: ! %processor %t/main.arda %t/second.arda -c -o %t/huh.o
// RUN: ! test -e %t/huh.o

// Multiple input compile-only no output
// RUN: %processor %t/main.arda %t/second.arda -c
// RUN: test -e %t/main.o && rm -rf %t/main.o
// RUN: test -e %t/second.o && rm -rf %t/second.o

// Multiple object and arda input compile-only no output
// RUN: %processor %t/main.arda %t/second.o -c
// RUN: test -e %t/main.o && rm -rf %t/main.o
// - Note second.o is untouched.

// Multiple input compile-only no output, same name
// RUN: ! %processor %t/main.o %t/main.arda -c
// - Note that this only fails because we take the "main"
//   from the first object file and try to use it as the
//   "main.o" output name of the second, which is the same
//   as the first paramter..

// Multiple object input compile-only no output
// RUN: ! %processor %t/main.o %t/second.o -c

//
// -- Single input compile and link --
//

// Single input compile-link no output
// RUN: %processor %t/main.arda
// RUN: test -e %t/main && rm -rf %t/main
// RUN: test -e %t/.eru/main.o && rm -rf %t/.eru/main.o

// Single object input compile-link no output
// - Note first we need to create the objectFile
// RUN: %processor %t/main.arda -c -o %t/test.o
// RUN: %processor %t/test.o
// RUN: test -e %t/test && rm -rf %t/test

// Single input compile-link with output
// RUN: %processor %t/main.arda -o %t/program
// RUN: test -e %t/program && rm -rf %t/program

// Single input compile-link with output, same name
// RUN: ! %processor %t/main.arda -o %t/main.arda

// Single object input compile-link with output
// RUN: %processor %t/test.o -o %t/test
// RUN: test -e %t/test && rm -rf %t/test

//
// -- Multiple input compile and link --
//

// Multiple input compile-link with output
// RUN: %processor %t/main.arda %t/second.arda -o %t/program
// RUN: test -e %t/program && rm -rf %t/program

// Multiple input compile-link with output, same name
// RUN: ! %processor %t/main.arda %t/second.arda -o %t/main.arda

// Multiple input compile-link no output
// RUN: %processor %t/main.arda %t/second.arda
// RUN: test -e %t/main && rm -rf %t/main

// Multiple object input compile-link no output
// - Note first we need to create another objectFile
// RUN: %processor %t/second.arda -c -o %t/test2.o
// RUN: %processor %t/test.o %t/test2.o
// RUN: test -e %t/test && rm -rf %t/test

// Multiple object and arda input compile-only no output
// RUN: %processor %t/main.arda %t/test2.o
// RUN: test -e %t/main && rm -rf %t/main

// Multiple object input compile-link with output
// RUN: %processor %t/second.arda -c -o %t/test2.o
// RUN: %processor %t/test.o %t/test2.o -o %t/program
// RUN: test -e %t/program && rm -rf %t/program

// Multiple object and arda input compile-only with output
// RUN: %processor %t/main.arda %t/test2.o -o %t/program
// RUN: test -e %t/program && rm -rf %t/program

//--- main.arda

int Valinor(int argc)[] { return argc + 42 }

//--- second.arda

int something()[] { return 33 }
