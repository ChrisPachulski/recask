/* Register contract, reads past the results of calls with no callee to mark: _call_blr reads the x2 _t set through blr, _call_stub
   the x2 libindl's _lset2 set through its stub, so those sites must reload every register. _hli does the blr read with x19 set
   under a save, and _call_blrcs reloads x19 past _widei (which reads x9, so its call stores and reloads every register): the site
   that stored x19 makes _hli store it back at its returns. */
#include <stdio.h>
#ifdef __aarch64__
long call_blr(long);
long call_stub(long);
long call_blrcs(long);
__asm__(".text\n.p2align 2\n"
        "_t:\n"
        "  mov x2, #42\n"
        "  ret\n"
        ".globl _call_blr\n"
        "_call_blr:\n"
        "  stp x29, x30, [sp, #-16]!\n"
        "  mov x29, sp\n"
        "  adrp x8, _t@PAGE\n"
        "  add x8, x8, _t@PAGEOFF\n"
        "  mov x2, #7\n"
        "  blr x8\n"
        "  add x0, x0, x2\n"
        "  ldp x29, x30, [sp], #16\n"
        "  ret\n"
        ".globl _call_stub\n"
        "_call_stub:\n"
        "  stp x29, x30, [sp, #-16]!\n"
        "  mov x29, sp\n"
        "  mov x2, #7\n"
        "  bl _lset2\n"
        "  add x0, x0, x2\n"
        "  ldp x29, x30, [sp], #16\n"
        "  ret\n"
        "_hli:\n"
        "  stp x19, x30, [sp, #-16]!\n"
        "  mov x19, #99\n"
        "  adrp x8, _t@PAGE\n"
        "  add x8, x8, _t@PAGEOFF\n"
        "  mov x2, #7\n"
        "  blr x8\n"
        "  mov x0, x2\n"
        "  ldp x19, x30, [sp], #16\n"
        "  ret\n"
        "_widei:\n"
        "  stp x29, x30, [sp, #-16]!\n"
        "  mov x29, sp\n"
        "  add x0, x0, x9\n"
        "  bl _hli\n"
        "  ldp x29, x30, [sp], #16\n"
        "  ret\n"
        ".globl _call_blrcs\n"
        "_call_blrcs:\n"
        "  stp x19, x30, [sp, #-16]!\n"
        "  add x19, x0, #7\n"
        "  mov x9, #0\n"
        "  bl _widei\n"
        "  mov x0, x19\n"
        "  ldp x19, x30, [sp], #16\n"
        "  ret\n");
#else
void lset2(void);
static long call_blr(long a) { return a + 42; }
static long call_stub(long a) { lset2(); return a + 42; }
static long call_blrcs(long a) { return a + 7; }
#endif
int main(void) {
    for (long a = 1; a <= 4; a++) printf("ind %ld -> %ld %ld %ld\n", a, call_blr(a), call_stub(a), call_blrcs(a));
    return 0;
}
