/* Register contract, a call into an image without it: libswl links Swift, so gdyld knows nothing of what its _lget9 reads at entry
   (x9, which no call passes). _call_g9 sets x9 and calls it through its stub, _call_p9 sets it to another value and calls _touch first, which
   leaves it alone: the stub's GOT site must store every register, and every call site a dirty ABI_NOT_IN one, as the code past
   the stub may read any of them. */
#include <stdio.h>
#ifdef __aarch64__
long call_g9(long);
long call_p9(long);
__asm__(".text\n.p2align 2\n"
        "_touch:\n"
        "  add x0, x0, #1\n"
        "  ret\n"
        ".globl _call_g9\n"
        "_call_g9:\n"
        "  stp x29, x30, [sp, #-16]!\n"
        "  mov x29, sp\n"
        "  add x9, x0, #9\n"
        "  bl _lget9\n"
        "  ldp x29, x30, [sp], #16\n"
        "  ret\n"
        ".globl _call_p9\n"
        "_call_p9:\n"
        "  stp x29, x30, [sp, #-16]!\n"
        "  mov x29, sp\n"
        "  add x9, x0, #19\n"
        "  bl _touch\n"
        "  bl _lget9\n"
        "  ldp x29, x30, [sp], #16\n"
        "  ret\n");
#else
long lget9(void);
static long call_g9(long a) { lget9(); return a + 9; }
static long call_p9(long a) { return a + 19; }
#endif
int main(void) {
    for (long a = 1; a <= 4; a++) printf("swl %ld -> %ld %ld\n", a, call_g9(a), call_p9(a));
    return 0;
}
