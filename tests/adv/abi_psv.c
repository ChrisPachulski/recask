/* Register contract, an entry value read after its prologue save: _get19 saves x19, returns the x19 it was entered with, and
   restores it. _call_s sets x19 and calls it: the read is the entry value's, the save does not make it the function's own. */
#include <stdio.h>
#ifdef __aarch64__
long call_s(long);
__asm__(".text\n.p2align 2\n"
        "_get19:\n"
        "  stp x19, x30, [sp, #-16]!\n"
        "  mov x0, x19\n"
        "  ldp x19, x30, [sp], #16\n"
        "  ret\n"
        "_call_s:\n"
        "  stp x19, x30, [sp, #-16]!\n"
        "  add x19, x0, #42\n"
        "  bl _get19\n"
        "  ldp x19, x30, [sp], #16\n"
        "  ret\n");
#else
static long call_s(long a) { return a + 42; }
#endif
int main(void) {
    for (long a = 1; a <= 4; a++) printf("psv %ld -> %ld\n", a, call_s(a));
    return 0;
}
