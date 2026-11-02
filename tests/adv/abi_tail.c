/* Register contract, a tail call is a return too: _tset9 sets x9 and _tsetv sets d16, which no call passes, then tail-call _touch,
   which leaves them alone; their callers pass them on to _use9 and _usev, which read them at entry (outside the contract). The tail
   call must store every dirty caller-saved register a return would, the CPU struct is where _use9 and _usev read them. */
#include <stdio.h>
#ifdef __aarch64__
long call_tail9(long);
long call_tailv(long);
__asm__(".text\n.p2align 2\n"
        "_touch:\n"
        "  add x0, x0, #1\n"
        "  ret\n"
        "_tset9:\n"
        "  add x9, x0, #9\n"
        "  b _touch\n"
        "_tsetv:\n"
        "  add x10, x0, #16\n"
        "  fmov d16, x10\n"
        "  b _touch\n"
        "_use9:\n"
        "  mov x0, x9\n"
        "  ret\n"
        "_usev:\n"
        "  fmov x0, d16\n"
        "  ret\n"
        ".globl _call_tail9\n"
        "_call_tail9:\n"
        "  stp x29, x30, [sp, #-16]!\n"
        "  mov x29, sp\n"
        "  bl _tset9\n"
        "  bl _use9\n"
        "  ldp x29, x30, [sp], #16\n"
        "  ret\n"
        ".globl _call_tailv\n"
        "_call_tailv:\n"
        "  stp x29, x30, [sp, #-16]!\n"
        "  mov x29, sp\n"
        "  bl _tsetv\n"
        "  bl _usev\n"
        "  ldp x29, x30, [sp], #16\n"
        "  ret\n");
#else
static long call_tail9(long a) { return a + 9; }
static long call_tailv(long a) { return a + 16; }
#endif
int main(void) {
    for (long a = 1; a <= 4; a++) printf("tail %ld -> %ld %ld\n", a, call_tail9(a), call_tailv(a));
    return 0;
}
