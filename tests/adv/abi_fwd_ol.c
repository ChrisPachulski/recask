/* Register contract, pass-through shapes: _OUTLINED_FUNCTION_0 hands back x2 past its results and its caller passes it on to _OUTLINED_FUNCTION_1;
   _OUTLINED_FUNCTION_2 leaves x9, which no call passes, alone and its caller passes the x9 it set before the call on to _OUTLINED_FUNCTION_3.
   Calls and returns must store every dirty caller-saved register, the next call reads them from the CPU struct.
   This copy names the helpers as the machine outliner names its bodies (_OUTLINED_FUNCTION_<n>), the only
   functions compiled code gives such a convention; abi_fwd keeps the hand-written names (a LIMIT). */
#include <stdio.h>
#ifdef __aarch64__
long call_set2(long);
long call_thru9(long);
__asm__(".text\n.p2align 2\n"
        "_OUTLINED_FUNCTION_0:\n"
        "  add x2, x0, #42\n"
        "  ret\n"
        "_OUTLINED_FUNCTION_1:\n"
        "  mov x0, x2\n"
        "  ret\n"
        "_OUTLINED_FUNCTION_2:\n"
        "  add x0, x0, #1\n"
        "  ret\n"
        "_OUTLINED_FUNCTION_3:\n"
        "  mov x0, x9\n"
        "  ret\n"
        ".globl _call_set2\n"
        "_call_set2:\n"
        "  stp x29, x30, [sp, #-16]!\n"
        "  mov x29, sp\n"
        "  mov x2, #7\n"
        "  bl _OUTLINED_FUNCTION_0\n"
        "  bl _OUTLINED_FUNCTION_1\n"
        "  ldp x29, x30, [sp], #16\n"
        "  ret\n"
        ".globl _call_thru9\n"
        "_call_thru9:\n"
        "  stp x29, x30, [sp, #-16]!\n"
        "  mov x29, sp\n"
        "  add x9, x0, #9\n"
        "  bl _OUTLINED_FUNCTION_2\n"
        "  bl _OUTLINED_FUNCTION_3\n"
        "  ldp x29, x30, [sp], #16\n"
        "  ret\n");
#else
static long call_set2(long a) { return a + 42; }
static long call_thru9(long a) { return a + 9; }
#endif
int main(void) {
    for (long a = 1; a <= 4; a++) printf("fwd %ld -> %ld %ld\n", a, call_set2(a), call_thru9(a));
    return 0;
}
