/* Register contract, a direct call into an extra entry point: _prim returns 11, and its interior entry (no symbol, no function
   start) returns the x19 it was entered with. _call_x sets x19 and calls the interior entry: the entry's own reads must reach
   its caller's site, as the primary entry's do. */
#include <stdio.h>
#ifdef __aarch64__
long prim(void);
long call_x(long);
__asm__(".text\n.p2align 2\n"
        "_prim:\n"
        "  mov x0, #11\n"
        "  ret\n"
        "Lxent:\n"
        "  mov x0, x19\n"
        "  ret\n"
        "_call_x:\n"
        "  stp x19, x30, [sp, #-16]!\n"
        "  add x19, x0, #42\n"
        "  bl Lxent\n"
        "  ldp x19, x30, [sp], #16\n"
        "  ret\n");
#else
static long prim(void) { return 11; }
static long call_x(long a) { return a + 42; }
#endif
int main(void) {
    for (long a = 1; a <= 4; a++) printf("xent %ld -> %ld %ld\n", a, prim(), call_x(a));
    return 0;
}
