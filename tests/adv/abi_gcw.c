/* Register contract, a wrapper of another image's export outside it: libgcw's _lget19 (abi_gcs_lib.c) reads x19 at entry.
   _call_w19 sets x19 and calls the local _wrap19, which calls _lget19 through its stub and leaves x19 alone: the GOT site stores
   every register, so the wrapper's caller must pass it the x19 the site stores. */
#include <stdio.h>
#ifdef __aarch64__
long call_w19(long);
__asm__(".text\n.p2align 2\n"
        "_wrap19:\n"
        "  stp x29, x30, [sp, #-16]!\n"
        "  mov x29, sp\n"
        "  bl _lget19\n"
        "  ldp x29, x30, [sp], #16\n"
        "  ret\n"
        ".globl _call_w19\n"
        "_call_w19:\n"
        "  stp x19, x30, [sp, #-16]!\n"
        "  add x19, x0, #7\n"
        "  bl _wrap19\n"
        "  ldp x19, x30, [sp], #16\n"
        "  ret\n");
#else
long lget19(void);
static long call_w19(long a) { lget19(); return a + 7; }
#endif
int main(void) {
    for (long a = 1; a <= 4; a++) printf("gcw %ld -> %ld\n", a, call_w19(a));
    return 0;
}
