// RUN: rm -rf %t
// RUN: %split-file %s %t

// Compile c program
// RUN: clang %t/main.c -o %t/main.o -c

// Manually link
// RUN: clang %t/main.o -o %t/main 

// RUN: %t/main a | FileCheck %s --check-prefix=NORMAL

// NORMAL: Hello a

// Compile main.arda
// RUN: %rewriter %t/main.o -o %t/main-mod.o

// Manually link modified object
// RUN clang %t/main-mod.o -o %t/main-mod

// RUN %t/main-mod

//--- main.c

#include <stdio.h>

int main(int argc, char *argv[]) {
  const char *aa = *(argv + 1);
  printf("Hello %s", aa);
  return 0;
}