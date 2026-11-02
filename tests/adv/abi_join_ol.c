/* Register contract, a join of a path that set a caller-saved register and one whose call left it unread: _join2 sets x2 itself
   on one branch and has _OUTLINED_FUNCTION_0 set it on the other (an -Oz outlined body's argument setup), then passes it to _OUTLINED_FUNCTION_1; _join9 does the
   same with x9 (a register outside the contract that _OUTLINED_FUNCTION_3 reads at entry); _loop9 sets x9, then calls _OUTLINED_FUNCTION_4 twice, which sets it
   on its first call only, then calls _OUTLINED_FUNCTION_3. Past each join the local is dirty on one path and stale on the other, where the CPU
   struct holds the callee's value: the store at the next call must not write the stale local over it.
   This copy names the helpers as the machine outliner names its bodies (_OUTLINED_FUNCTION_<n>), the only
   functions compiled code gives such a convention; abi_join keeps the hand-written names (a LIMIT). */
#include <stdio.h>
#ifdef __aarch64__
long join2(long);
long join9(long);
long loop9(long);
__asm__(".text\n.p2align 2\n"
        "_OUTLINED_FUNCTION_0:\n"
        "  add x2, x0, #40\n"
        "  ret\n"
        "_OUTLINED_FUNCTION_1:\n"
        "  mov x0, x2\n"
        "  ret\n"
        "_OUTLINED_FUNCTION_2:\n"
        "  add x9, x0, #40\n"
        "  ret\n"
        "_OUTLINED_FUNCTION_3:\n"
        "  mov x0, x9\n"
        "  ret\n"
        "_OUTLINED_FUNCTION_4:\n"
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
        "  bl _OUTLINED_FUNCTION_0\n"
        "2:\n"
        "  bl _OUTLINED_FUNCTION_1\n"
        "  ldp x29, x30, [sp], #16\n"
        "  ret\n"
        "_join9:\n"
        "  stp x29, x30, [sp, #-16]!\n"
        "  mov x29, sp\n"
        "  tbz x0, #0, 1f\n"
        "  add x9, x0, #7\n"
        "  b 2f\n"
        "1:\n"
        "  bl _OUTLINED_FUNCTION_2\n"
        "2:\n"
        "  bl _OUTLINED_FUNCTION_3\n"
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
        "  bl _OUTLINED_FUNCTION_4\n"
        "  add x20, x20, #1\n"
        "  cmp x20, #2\n"
        "  b.ne 1b\n"
        "  bl _OUTLINED_FUNCTION_3\n"
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
