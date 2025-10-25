int Valinor() {

    return 42;
}

extern int __libc_start_main(
    int (*main)(),
    int argc,
    char **argv,
    void (*init)(void),
    void (*fini)(void),
    void (*rtld_fini)(void),
    void *stack_end
);

void _start(void) {
    long *stack = __builtin_frame_address(0) + sizeof(void*);
    long argc = *stack;
    char **argv = (char **)(stack + 1);
    char **envp = argv + argc + 1;
    
    __libc_start_main(
        Valinor,           // main function
        argc,              // argc
        argv,              // argv
        0,              // init
        0,              // fini
        0,              // rtld_fini
        stack              // stack_end
    );
}