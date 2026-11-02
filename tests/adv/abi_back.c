/* Register contract, given-back callee-saved shape: _tailh saves x19, changes it, restores it and tail-calls _cs19,
   which reads x19; _callh does the same through a plain call. A restore ends the function's ownership of x19, so
   _cs19's need for its caller's x19 is its callers' need too, and their call sites must store x19. */
#include <stdio.h>
#ifdef __aarch64__
long call_tailh(long);
long call_callh(long);
__asm__(".text\n.p2align 2\n"
        "_cs19:\n"
        "  mov x0, x19\n"
        "  ret\n"
        "_tailh:\n"
        "  stp x19, x20, [sp, #-16]!\n"
        "  mov x19, #99\n"
        "  ldp x19, x20, [sp], #16\n"
        "  b _cs19\n"
        "_callh:\n"
        "  stp x19, x20, [sp, #-32]!\n"
        "  stp x29, x30, [sp, #16]\n"
        "  add x29, sp, #16\n"
        "  mov x19, #99\n"
        "  ldp x19, x20, [sp]\n"
        "  bl _cs19\n"
        "  ldp x29, x30, [sp, #16]\n"
        "  add sp, sp, #32\n"
        "  ret\n"
        ".globl _call_tailh\n"
        "_call_tailh:\n"
        "  stp x19, x30, [sp, #-16]!\n"
        "  add x19, x0, #7\n"
        "  bl _tailh\n"
        "  ldp x19, x30, [sp], #16\n"
        "  ret\n"
        ".globl _call_callh\n"
        "_call_callh:\n"
        "  stp x19, x30, [sp, #-16]!\n"
        "  add x19, x0, #11\n"
        "  bl _callh\n"
        "  ldp x19, x30, [sp], #16\n"
        "  ret\n");
#else
static long call_tailh(long a) { return a + 7; }
static long call_callh(long a) { return a + 11; }
#endif
int main(void) {
    for (long a = 1; a <= 4; a++) printf("back %ld -> %ld %ld\n", a, call_tailh(a), call_callh(a));
    return 0;
}
