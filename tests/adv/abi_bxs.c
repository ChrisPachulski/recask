/* Register contract, a raw indirect call into another image's export outside it: libbxs's _lget19 (abi_gcs_lib.c) reads x19 at
   entry. _call_b19 sets x19 and calls the pointer main passes it, with no GOT slot at the site: an indirect call may reach any
   function a pointer or an export reaches, so it must store every register. */
#include <stdio.h>
#ifdef __aarch64__
long lget19(void);
long call_b19(long, long (*)(void));
__asm__(".text\n.p2align 2\n"
        ".globl _call_b19\n"
        "_call_b19:\n"
        "  stp x19, x30, [sp, #-16]!\n"
        "  add x19, x0, #7\n"
        "  blr x1\n"
        "  ldp x19, x30, [sp], #16\n"
        "  ret\n");
#else
long lget19(void);
static long call_b19(long a, long (*f)(void)) { (void)f; return a + 7; }
#endif
int main(void) {
    for (long a = 1; a <= 4; a++) printf("bxs %ld -> %ld\n", a, call_b19(a, lget19));
    return 0;
}
