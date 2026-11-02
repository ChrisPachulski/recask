/* Register contract, a call into another image's export outside it: libgcs's _lget19 reads x19 at entry. _call_g19 sets x19 and
   calls it through its stub, _call_p19 sets it to another value and calls _touch first: the stub's GOT site must store every register, x19 (a
   callee-saved one, kept in a local across _touch) included. */
#include <stdio.h>
#ifdef __aarch64__
long call_g19(long);
long call_p19(long);
__asm__(".text\n.p2align 2\n"
        "_touch:\n"
        "  add x0, x0, #1\n"
        "  ret\n"
        ".globl _call_g19\n"
        "_call_g19:\n"
        "  stp x19, x30, [sp, #-16]!\n"
        "  add x19, x0, #7\n"
        "  bl _lget19\n"
        "  ldp x19, x30, [sp], #16\n"
        "  ret\n"
        ".globl _call_p19\n"
        "_call_p19:\n"
        "  stp x19, x30, [sp, #-16]!\n"
        "  add x19, x0, #17\n"
        "  bl _touch\n"
        "  bl _lget19\n"
        "  ldp x19, x30, [sp], #16\n"
        "  ret\n");
#else
long lget19(void);
static long call_g19(long a) { lget19(); return a + 7; }
static long call_p19(long a) { return a + 17; }
#endif
int main(void) {
    for (long a = 1; a <= 4; a++) printf("gcs %ld -> %ld %ld\n", a, call_g19(a), call_p19(a));
    return 0;
}
