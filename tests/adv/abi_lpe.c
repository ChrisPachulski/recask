/* Register contract, a loop's entry spill past a call that left a register unread: _lpe sets x2, then loops calling _set2 (which
   sets x2 for its caller, as an -Oz outlined body may, and clears x0) until x0 is 0, then calls _use2, which returns x2. The
   spill on the edge into the second loop stores x2 although every path there has already stored it or left it to a callee:
   the call before it must reload x2 so the store writes the callee's value, not the one set before the first loop. */
#include <stdio.h>
#ifdef __aarch64__
long lpe(long);
__asm__(".text\n.p2align 2\n"
        "_set2:\n"
        "  add x2, x0, #40\n"
        "  mov x0, #0\n"
        "  ret\n"
        "_use2:\n"
        "  mov x0, x2\n"
        "  ret\n"
        "_lpe:\n"
        "  stp x19, x30, [sp, #-16]!\n"
        "  mov x19, x0\n"
        "  add x2, x0, #7\n"
        "  mov x0, #1\n"
        "1:\n"
        "  cbz x0, 2f\n"
        "  mov x0, x19\n"
        "  bl _set2\n"
        "  b 1b\n"
        "2:\n"
        "  bl _use2\n"
        "  cbz x0, 2b\n"
        "  ldp x19, x30, [sp], #16\n"
        "  ret\n");
#else
static long lpe(long a) { return a + 40; }
#endif
int main(void) {
    for (long a = 1; a <= 4; a++) printf("lpe %ld -> %ld\n", a, lpe(a));
    return 0;
}
