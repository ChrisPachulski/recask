/* abi_gcs's guest library: the exported _lget19 returns the x19 it was entered with, which no call passes (its own image keeps
   full register sync, as an export reaches it). */
#ifdef __aarch64__
__asm__(".text\n.p2align 2\n"
        ".globl _lget19\n"
        "_lget19:\n"
        "  mov x0, x19\n"
        "  ret\n");
#else
long lget19(void) { return 0; }
#endif
