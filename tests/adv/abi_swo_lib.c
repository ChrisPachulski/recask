/* abi_swo's guest library, linked against Swift so it runs without the register contract (gdyld abi_ok): _lset21 leaves x21
   changed, as a Swift callee returning an error does. */
#ifdef __aarch64__
__asm__(".text\n.p2align 2\n"
        ".globl _lset21\n"
        "_lset21:\n"
        "  add x21, x0, #40\n"
        "  ret\n");
#else
long lset21(long a) { return a; }
#endif
