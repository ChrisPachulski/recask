/* Register contract, a callee-saved register an image without it changes, through a wrapper: libswo links Swift, and its _lset21
   sets x21 (as Swift returns an error). _call_w21 sets x21, calls the local _wrap21, which calls _lset21 through its stub, and
   returns x21: the wrapper's caller must reload it, as the unknown callee may change any register. */
#include <stdio.h>
#ifdef __aarch64__
long call_w21(long);
__asm__(".text\n.p2align 2\n"
        "_wrap21:\n"
        "  stp x29, x30, [sp, #-16]!\n"
        "  mov x29, sp\n"
        "  bl _lset21\n"
        "  ldp x29, x30, [sp], #16\n"
        "  ret\n"
        ".globl _call_w21\n"
        "_call_w21:\n"
        "  stp x21, x30, [sp, #-16]!\n"
        "  add x21, x0, #7\n"
        "  bl _wrap21\n"
        "  mov x0, x21\n"
        "  ldp x21, x30, [sp], #16\n"
        "  ret\n");
#else
long lset21(long);
static long call_w21(long a) { lset21(a); return a + 40; }
#endif
int main(void) {
    for (long a = 1; a <= 4; a++) printf("swo %ld -> %ld\n", a, call_w21(a));
    return 0;
}
