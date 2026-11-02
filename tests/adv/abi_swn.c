/* Register contract, a call into an image without it whose callee has no NZCV facts: libswn links Swift, and its _ladd9 returns
   the x9 it was entered with plus one, setting flags no one reads, so gdyld's flag rows hold nothing of it. _call_a9 sets x9 and
   calls it through its stub: the census of the image's functions must still find it (the GOT site stores every register). */
#include <stdio.h>
#ifdef __aarch64__
long call_a9(long);
__asm__(".text\n.p2align 2\n"
        ".globl _call_a9\n"
        "_call_a9:\n"
        "  stp x29, x30, [sp, #-16]!\n"
        "  mov x29, sp\n"
        "  add x9, x0, #9\n"
        "  bl _ladd9\n"
        "  ldp x29, x30, [sp], #16\n"
        "  ret\n");
#else
long ladd9(void);
static long call_a9(long a) { ladd9(); return a + 10; }
#endif
int main(void) {
    for (long a = 1; a <= 4; a++) printf("swn %ld -> %ld\n", a, call_a9(a));
    return 0;
}
