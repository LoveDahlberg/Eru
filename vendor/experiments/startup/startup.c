// This is the minimal amount of code needed to have the _start function (aka
// the entry point for ELF-x86). The operating system has already set the stack
// up for us.
//
// Alternatively, we could have used __libc_start_main, which would provide us
// with the following:
// - Passing of argv, argc and envp  (pointer to environment variables)
// - A pointer to global constructor which would run automatically.
// - A pointer to global destructor which would run automatically.
// - A pointer to a dynamic linker cleanup function.
// - and more
//
// But as we are just looking for the bare minimum to start with, the code below
// will work.
//
// Clang compiler flags:
// -nostdlib : links no startup funcionality and no libc.
// -nostartfiles : links no startup functtionality.
// -lc : links libc only.
//
// So what we want to do is create an object file that contains the below _start
// function and then calls the Valinor function. The eru compiler will link
// against this object when called without the -c flag (aka when they want to
// compile and link a program). When linking this object file, we have to pass
// nostdlib.
//
// The object file should be in C for now. And the file being linked against is
// created from eru. Note, to start with, vectors are not supported in eru, so
// we can start by just creating a Valinor function with single argc, then
// expand it to also look for a Valinor without any args, and then add argv
// (unless we go with another way of getting the args entirely of course).

//--- startup.c

// Forward declaration just for show, it will be needed in the final object file
// though.
int Valinor(int argc, char **argv);

// Set as naked so that clang doesn't add a proluge (or epilogue) which makes it
// more annoying to find argv and argc
__attribute__((naked)) void _start(void) {
  // When called, the stack will look like the following:
  //
  // (environment variables, which we dont care about now)
  // [NULL]
  // [n-1th argv pointer]
  // [1st argv pointer]
  // [0th argv pointer]
  // [argc]  <- position of stack pointer rsp, set beforehand.
  // (here is essentially where the stack starts and where rbp would be saved)
  //
  // Now with the proluge, we would add rbp and make room (underneath argc) for
  // some other stuff being passed. What exactly happens here is compiler
  // specific (depending on optimization level and compiler version gcc vs clang
  // etc). This is an unknown we don't want here for now, so we keep the
  // fucntion naked to avoid any automatic code being added.

  // So here we essentially move argc (where rsp points to) into the first
  // argument (rdi) and argv (8 bytes above rsp) into second argument (rsi) and
  // then we call Valinor.
  // After the call, we move the return value (in eax) into a the syscall
  // argument register (edi). Then we set the syscall number 60 (in eax) and
  // call syscall.
  //
  // This way of exiting is not ordinary, but is enough for now.

  __asm__ volatile(
      "movq (%%rsp), %%rdi\n\t"  // Load argc directly into rdi (1st argument)
      "leaq 8(%%rsp), %%rsi\n\t" // Load address of argv into rsi (2nd argument)
      "call Valinor\n\t"         // Call Valinor, result will be in eax
      "movl %%eax, %%edi\n\t"    // Move result to edi for exit syscall
      "movl $60, %%eax\n\t"      // Syscall number 60
      "syscall\n\t"              // Exit
      :
      :
      :);
}

//--- main.c

// Test Valinor function which is in another object file.
int Valinor(int argc, char **argv) { return argc; }