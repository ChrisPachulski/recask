/* Register contract, a loop entry's store: _call_loop9 sets x9, which no call passes, before a loop that calls _OUTLINED_FUNCTION_0 and then
   _OUTLINED_FUNCTION_1, which reads it at entry (outside the contract). The loop's entry edge stores what the loop never writes once, and the calls
   in it count on that: under the contract the edge must store every register a later call or return would, x9 included.
   This copy names the helpers as the machine outliner names its bodies (_OUTLINED_FUNCTION_<n>), the only
   functions compiled code gives such a convention; abi_edge keeps the hand-written names (a LIMIT). */
#include <stdio.h>
#ifdef __aarch64__
long call_loop9(long);
__asm__(".text\n.p2align 2\n"
        "_OUTLINED_FUNCTION_0:\n"
        "  add x0, x0, #1\n"
        "  ret\n"
        "_OUTLINED_FUNCTION_1:\n"
        "  mov x0, x9\n"
        "  ret\n"
        ".globl _call_loop9\n"
        "_call_loop9:\n"
        "  stp x29, x30, [sp, #-16]!\n"
        "  mov x29, sp\n"
        "  stp x19, x20, [sp, #-16]!\n"
        "  add x9, x0, #9\n"
        "  mov x19, #3\n"
        "  mov x20, #0\n"
        "1:\n"
        "  bl _OUTLINED_FUNCTION_0\n"
        "  bl _OUTLINED_FUNCTION_1\n"
        "  add x20, x20, x0\n"
        "  subs x19, x19, #1\n"
        "  b.ne 1b\n"
        "  mov x0, x20\n"
        "  ldp x19, x20, [sp], #16\n"
        "  ldp x29, x30, [sp], #16\n"
        "  ret\n");
#else
static long call_loop9(long a) { return 3 * (a + 9); }
#endif
int main(void) {
    for (long a = 1; a <= 4; a++) printf("edge %ld -> %ld\n", a, call_loop9(a));
    return 0;
}
