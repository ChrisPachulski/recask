/* Register contract, kept caller-saved shape: _keep2 saves x2, changes it across its own call and restores it, and
   its caller hands x2 on to _use2 without setting it again, as machine-outlined code relies on registers a callee
   leaves alone; _keep9 does the same with x9, which no call passes, around a call to _use9 that reads it. A callee
   that gives a caller-saved register back from its save slot must store it at its return. */
#include <stdio.h>
#ifdef __aarch64__
long call_keep2(long);
long call_keep9(long);
__asm__(".text\n.p2align 2\n"
        "_nop:\n"
        "  ret\n"
        "_keep2:\n"
        "  stp x2, x30, [sp, #-16]!\n"
        "  mov x2, #99\n"
        "  bl _nop\n"
        "  ldp x2, x30, [sp], #16\n"
        "  ret\n"
        "_use2:\n"
        "  mov x0, x2\n"
        "  ret\n"
        "_keep9:\n"
        "  stp x9, x30, [sp, #-16]!\n"
        "  mov x9, #99\n"
        "  bl _use9\n"
        "  ldp x9, x30, [sp], #16\n"
        "  ret\n"
        "_use9:\n"
        "  mov x0, x9\n"
        "  ret\n"
        ".globl _call_keep2\n"
        "_call_keep2:\n"
        "  stp x29, x30, [sp, #-16]!\n"
        "  mov x29, sp\n"
        "  add x2, x0, #7\n"
        "  bl _keep2\n"
        "  bl _use2\n"
        "  ldp x29, x30, [sp], #16\n"
        "  ret\n"
        ".globl _call_keep9\n"
        "_call_keep9:\n"
        "  stp x29, x30, [sp, #-16]!\n"
        "  mov x29, sp\n"
        "  add x9, x0, #9\n"
        "  bl _keep9\n"
        "  bl _use9\n"
        "  ldp x29, x30, [sp], #16\n"
        "  ret\n");
#else
static long call_keep2(long a) { return a + 7; }
static long call_keep9(long a) { return a + 9; }
#endif
int main(void) {
    for (long a = 1; a <= 4; a++) printf("keep %ld -> %ld %ld\n", a, call_keep2(a), call_keep9(a));
    return 0;
}
