// clang-format off

// Test that the rewriter can read in and correctly output the same object file it ingested.

// RUN: rm -rf %t
// RUN: %split-file %s %t

// Compile the example and pass it into the rewriter.
// RUN: clang %t/main.c -o %t/main.o -c
// RUN: %rewriter %t/main.o -o %t/main-mod.o

// Check the object file raw dump with relocations, before and after processing.
// RUN objdump -r -d %t/main-mod.o | FileCheck %s --check-prefix=DUMP
// RUN objdump -r -d %t/main.o | FileCheck %s --check-prefix=DUMP
//
// DUMP: 0000000000000000 <main>:
// DUMP:    0:	55                   	push   %rbp
// DUMP:    1:	48 89 e5             	mov    %rsp,%rbp
// DUMP:    4:	48 83 ec 20          	sub    $0x20,%rsp
// DUMP:    8:	c7 45 fc 00 00 00 00 	movl   $0x0,-0x4(%rbp)
// DUMP:    f:	89 7d f8             	mov    %edi,-0x8(%rbp)
// DUMP:   12:	48 89 75 f0          	mov    %rsi,-0x10(%rbp)
// DUMP:   16:	48 8b 45 f0          	mov    -0x10(%rbp),%rax
// DUMP:   1a:	48 8b 40 08          	mov    0x8(%rax),%rax
// DUMP:   1e:	48 89 45 e8          	mov    %rax,-0x18(%rbp)
// DUMP:   22:	48 8b 75 e8          	mov    -0x18(%rbp),%rsi
// DUMP:   26:	48 8d 3d 00 00 00 00 	lea    0x0(%rip),%rdi        # 2d <main+0x2d>
// DUMP: 			29: R_X86_64_PC32	.L.str-0x4
// DUMP:   2d:	b0 00                	mov    $0x0,%al
// DUMP:   2f:	e8 00 00 00 00       	call   34 <main+0x34>
// DUMP: 			30: R_X86_64_PLT32	printf-0x4
// DUMP:   34:	31 c0                	xor    %eax,%eax
// DUMP:   36:	48 83 c4 20          	add    $0x20,%rsp
// DUMP:   3a:	5d                   	pop    %rbp
// DUMP:   3b:	c3                   	ret    

// TODO: Doesnt fully match exactly.
//
// Check relocations and symtab. 
// RUN readelf -a %t/main-mod.o | FileCheck %s --check-prefix=META
// RUN readelf -a %t/main.o | FileCheck %s --check-prefix=META
//
// META:   Offset          Info           Type             Sym. Value    Sym. Name + Addend
// META: 000000000029     {{.*}}      R_X86_64_PC32     0000000000000000 .L.str - 4
// META: 000000000030     {{.*}}      R_X86_64_PLT32    0000000000000000 printf - 4
//
// META:    Num:    Value          Size      Type    Bind   Vis      Ndx Name
// META:      0: 0000000000000000     0      NOTYPE  LOCAL  DEFAULT  UND 
// META: {{.*}}: 0000000000000000     9      OBJECT  LOCAL  DEFAULT    4 .L.str
// META: {{.*}}: 0000000000000000     60     FUNC    GLOBAL DEFAULT    2 main
// META: {{.*}}: 0000000000000000     0      NOTYPE  GLOBAL DEFAULT  UND printf

// Link and run both the original and the rewritten object file.
// RUN: clang %t/main.o -o %t/main 
// RUN: clang %t/main-mod.o -o %t/main-mod
//
// RUN: %t/main World! | FileCheck %s --check-prefix=PRINT
// RUN: %t/main-mod World! | FileCheck %s --check-prefix=PRINT

// PRINT: Hello World!

//--- main.c
// clang-format on

#include <stdio.h>

int main(int argc, char *argv[]) {
  const char *aa = *(argv + 1);
  printf("Hello %s", aa);
  return 0;
}
