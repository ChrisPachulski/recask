/* Register contract, a join of a path that set a caller-saved register and one whose call left it unread: _join2 sets x2 itself
   on one branch and has _set2 set it on the other (an -Oz outlined body's argument setup), then passes it to _use2; _join9 does the
   same with x9 (a register outside the contract that _use9 reads at entry); _loop9 sets x9, then calls _once9 twice, which sets it
   on its first call only, then calls _use9. Past each join the local is dirty on one path and stale on the other, where the CPU
   struct holds the callee's value: the store at the next call must not write the stale local over it. */
#include <stdio.h>
#ifdef __aarch64__
long join2(long);
long join9(long);
long loop9(long);
__asm__(".text\n.p2align 2\n"
        "_set2:\n"
        "  add x2, x0, #40\n"
        "  ret\n"
        "_use2:\n"
        "  mov x0, x2\n"
        "  ret\n"
        "_set9:\n"
        "  add x9, x0, #40\n"
        "  ret\n"
        "_use9:\n"
        "  mov x0, x9\n"
        "  ret\n"
        "_once9:\n"
        "  cbnz x1, 1f\n"
        "  add x9, x0, #40\n"
        "1:\n"
        "  ret\n"
        "_join2:\n"
        "  stp x29, x30, [sp, #-16]!\n"
        "  mov x29, sp\n"
        "  tbz x0, #0, 1f\n"
        "  add x2, x0, #7\n"
        "  b 2f\n"
        "1:\n"
        "  bl _set2\n"
        "2:\n"
        "  bl _use2\n"
        "  ldp x29, x30, [sp], #16\n"
        "  ret\n"
        "_join9:\n"
        "  stp x29, x30, [sp, #-16]!\n"
        "  mov x29, sp\n"
        "  tbz x0, #0, 1f\n"
        "  add x9, x0, #7\n"
        "  b 2f\n"
        "1:\n"
        "  bl _set9\n"
        "2:\n"
        "  bl _use9\n"
        "  ldp x29, x30, [sp], #16\n"
        "  ret\n"
        "_loop9:\n"
        "  stp x20, x19, [sp, #-32]!\n"
        "  stp x29, x30, [sp, #16]\n"
        "  add x29, sp, #16\n"
        "  mov x19, x0\n"
        "  add x9, x0, #7\n"
        "  mov x20, #0\n"
        "1:\n"
        "  mov x0, x19\n"
        "  mov x1, x20\n"
        "  bl _once9\n"
        "  add x20, x20, #1\n"
        "  cmp x20, #2\n"
        "  b.ne 1b\n"
        "  bl _use9\n"
        "  ldp x29, x30, [sp, #16]\n"
        "  ldp x20, x19, [sp], #32\n"
        "  ret\n");
#else
static long join2(long a) { return a & 1 ? a + 7 : a + 40; }
static long join9(long a) { return a & 1 ? a + 7 : a + 40; }
static long loop9(long a) { return a + 40; }
#endif
int main(void) {
    for (long a = 1; a <= 4; a++) printf("join %ld -> %ld %ld %ld\n", a, join2(a), join9(a), loop9(a));
    return 0;
}
