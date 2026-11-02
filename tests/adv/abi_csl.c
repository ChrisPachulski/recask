/* Register contract, a store at a call site that must leave callee-saved registers alone: _hand sets x19 under a save and calls
   _set2, whose x2 _use2 reads, so that site stores the caller-saved registers _set2 hands on; _wide reads x9, which no call passes,
   so _call_wide's call to it stores and reloads every register, x19 too. No return stores x19, so the CPU struct must still hold
   the x19 _call_wide stored when it reloads it: a site storing _hand's x19 there hands _call_wide 99. _call_wide sets every
   register an unknown callee may read past the results (x2-x8, v4-v7) itself, so main's printf reads none of _hand's and no
   return between stores x19 back. */
#include <stdio.h>
#ifdef __aarch64__
long call_wide(long);
__asm__(".text\n.p2align 2\n"
        "_set2:\n"
        "  add x2, x0, #42\n"
        "  ret\n"
        "_use2:\n"
        "  mov x0, x2\n"
        "  ret\n"
        "_hand:\n"
        "  stp x19, x30, [sp, #-16]!\n"
        "  mov x19, #99\n"
        "  bl _set2\n"
        "  bl _use2\n"
        "  ldp x19, x30, [sp], #16\n"
        "  ret\n"
        "_wide:\n"
        "  stp x29, x30, [sp, #-16]!\n"
        "  mov x29, sp\n"
        "  add x0, x0, x9\n"
        "  bl _hand\n"
        "  ldp x29, x30, [sp], #16\n"
        "  ret\n"
        ".globl _call_wide\n"
        "_call_wide:\n"
        "  stp x19, x30, [sp, #-16]!\n"
        "  add x19, x0, #7\n"
        "  mov x9, #0\n"
        "  bl _wide\n"
        "  mov x2, #0\n  mov x3, #0\n  mov x4, #0\n  mov x5, #0\n  mov x6, #0\n  mov x7, #0\n  mov x8, #0\n"
        "  movi d4, #0\n  movi d5, #0\n  movi d6, #0\n  movi d7, #0\n"
        "  mov x0, x19\n"
        "  ldp x19, x30, [sp], #16\n"
        "  ret\n");
#else
static long call_wide(long a) { return a + 7; }
#endif
int main(void) {
    for (long a = 1; a <= 4; a++) printf("csl %ld -> %ld\n", a, call_wide(a));
    return 0;
}
