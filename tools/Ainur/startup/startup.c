

// TODO: Right now this will try to call any symbol called valinor regardless of parameters. 
// Change this to instead be handled by the compiler itself. It should recognize and generate 
// the correct startup code calling the correct Valinor.

int Valinor(int argc);

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
  // argument (rdi) and then we call Valinor. When we need argv, get it 8 bytes
  // above rsp and put it into the second argument (rsi).
  //
  // After the call, we move the return value (in eax) into a the syscall
  // argument register (edi). Then we set the syscall number 60 (in eax) and
  // call syscall.
  //
  // This way of exiting is not ordinary, but is enough for now.

  __asm__ volatile(
      "movq (%%rsp), %%rdi\n\t" // Load argc directly into rdi
      "call Valinor\n\t"        // Call Valinor, result will be in eax
      "movl %%eax, %%edi\n\t"   // Move result to edi for exit syscall
      "movl $60, %%eax\n\t"     // Syscall number 60
      "syscall\n\t"             // Exit
      :
      :
      :);
}