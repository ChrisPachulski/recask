/* Register contract, a call site past an undecoded br: _helper saves x19, sets it and x9, and jumps through a pointer it loads into
   its own block (an undecoded br), which calls _use9 (it reads x9 at entry, so the site stores and reloads everything, x19 included) and restores x19.
   Its caller _wrap reads x9 at entry too (_outer sets it), so _outer's call into it reloads x19 after: _helper's returns must store the x19 it
   restored, as its site stored its own value there. */
#include <stdio.h>
#ifdef __aarch64__
long outer(long);
__asm__(".text\n.p2align 2\n"
        "_use9:\n"
        "  mov x0, x9\n"
        "  ret\n"
        "_helper:\n"
        "  stp x19, x30, [sp, #-16]!\n"
        "  mov x19, #99\n"
        "  mov x9, #5\n"
        "  adrp x1, Lbrx@PAGE\n"
        "  ldr x1, [x1, Lbrx@PAGEOFF]\n"
        "  br x1\n"
        "1:\n"
        "  bl _use9\n"
        "  ldp x19, x30, [sp], #16\n"
        "  ret\n"
        "_wrap:\n"
        "  stp x29, x30, [sp, #-16]!\n"
        "  mov x29, sp\n"
        "  mov x10, x9\n"
        "  bl _helper\n"
        "  ldp x29, x30, [sp], #16\n"
        "  ret\n"
        "_outer:\n"
        "  stp x19, x30, [sp, #-16]!\n"
        "  add x19, x0, #7\n"
        "  mov x9, #3\n"
        "  bl _wrap\n"
        "  mov x0, x19\n"
        "  ldp x19, x30, [sp], #16\n"
        "  ret\n"
        ".data\n.p2align 3\n"
        "Lbrx:\n"
        "  .quad 1b\n"
        ".text\n");
#else
static long outer(long a) { return a + 7; }
#endif
int main(void) {
    for (long a = 1; a <= 4; a++) printf("brx %ld -> %ld\n", a, outer(a));
    return 0;
}
