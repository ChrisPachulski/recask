/* abi_swl's guest library, linked against Swift so it runs without the register contract (gdyld abi_ok) and collects no register
   facts: _lget9 returns the x9 it was entered with. */
#ifdef __aarch64__
__asm__(".text\n.p2align 2\n"
        ".globl _lget9\n"
        "_lget9:\n"
        "  mov x0, x9\n"
        "  ret\n");
#else
long lget9(void) { return 0; }
#endif
