
// Test program that calls the Valinor (the main) of a linked Eru program.
// For now, it just forwards the first argument as an int to the program.
// If no argument is given, -1 is passed. On exit, the status code is printed
// so that it can be FileChecked if needed.

#include <stdio.h>
#include <stdlib.h>

int Valinor(int firstCommandlineArgument);

int main(int argc, char **argv) {

  int argument = -1;
  if (argc >= 2) {
    argument = atoi(argv[1]);
  }
  printf("Calling with %d", argument);
  int returnStatus = Valinor(argument);

  printf("Exit Code: %d", returnStatus);

  return 0;
}