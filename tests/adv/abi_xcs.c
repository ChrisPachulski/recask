/* Register contract, a full store at a call into a function outside it: _hx sets x19 under a save and calls _use9, which reads x9
   at entry, so that site stores every dirty register, x19 too; _wide reads x9 as well, so _call_xcs's call to it stores and reloads
   every register. _hx restores x19 and returns: the CPU struct must hold _call_xcs's x19 again when it reloads it, so a function
   whose own call site stored its callee-saved registers stores them back at its returns. */
#include <stdio.h>
#ifdef __aarch64__
long call_xcs(long);
__asm__(".text\n.p2align 2\n"
        "_use9:\n"
        "  mov x0, x9\n"
        "  ret\n"
        "_hx:\n"
        "  stp x19, x30, [sp, #-16]!\n"
        "  mov x19, #99\n"
        "  mov x9, #5\n"
        "  bl _use9\n"
        "  ldp x19, x30, [sp], #16\n"
        "  ret\n"
        "_wide:\n"
        "  stp x29, x30, [sp, #-16]!\n"
        "  mov x29, sp\n"
        "  add x0, x0, x9\n"
        "  bl _hx\n"
        "  ldp x29, x30, [sp], #16\n"
        "  ret\n"
        ".globl _call_xcs\n"
        "_call_xcs:\n"
        "  stp x19, x30, [sp, #-16]!\n"
        "  add x19, x0, #7\n"
        "  mov x9, #0\n"
        "  bl _wide\n"
        "  mov x0, x19\n"
        "  ldp x19, x30, [sp], #16\n"
        "  ret\n");
#else
static long call_xcs(long a) { return a + 7; }
#endif
int main(void) {
    for (long a = 1; a <= 4; a++) printf("xcs %ld -> %ld\n", a, call_xcs(a));
    return 0;
}
