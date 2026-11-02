/* Register contract, a site that reads past its callee's results: _hl sets x19 under a save and calls _get2, whose x2 it reads
   after the call, so that site stores and reloads every register, x19 too; _wide reads x9, which no call passes, so _call_lcs's
   call to it stores and reloads every register. _hl restores x19 and returns: the CPU struct must hold _call_lcs's x19 again when
   it reloads it, so a function with such a site stores its callee-saved registers back at its returns. */
#include <stdio.h>
#ifdef __aarch64__
long call_lcs(long);
__asm__(".text\n.p2align 2\n"
        "_get2:\n"
        "  mov x2, #42\n"
        "  ret\n"
        "_hl:\n"
        "  stp x19, x30, [sp, #-16]!\n"
        "  mov x19, #99\n"
        "  bl _get2\n"
        "  mov x0, x2\n"
        "  ldp x19, x30, [sp], #16\n"
        "  ret\n"
        "_wide:\n"
        "  stp x29, x30, [sp, #-16]!\n"
        "  mov x29, sp\n"
        "  add x0, x0, x9\n"
        "  bl _hl\n"
        "  ldp x29, x30, [sp], #16\n"
        "  ret\n"
        ".globl _call_lcs\n"
        "_call_lcs:\n"
        "  stp x19, x30, [sp, #-16]!\n"
        "  add x19, x0, #7\n"
        "  mov x9, #0\n"
        "  bl _wide\n"
        "  mov x0, x19\n"
        "  ldp x19, x30, [sp], #16\n"
        "  ret\n");
#else
static long call_lcs(long a) { return a + 7; }
#endif
int main(void) {
    for (long a = 1; a <= 4; a++) printf("lcs %ld -> %ld\n", a, call_lcs(a));
    return 0;
}
