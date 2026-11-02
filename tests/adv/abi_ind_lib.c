/* abi_ind's guest library: _lset2 hands back x2 past its result, which its caller reads after the stub call. */
#ifdef __aarch64__
__asm__(".text\n.p2align 2\n"
        ".globl _lset2\n"
        "_lset2:\n"
        "  mov x2, #42\n"
        "  ret\n");
#else
void lset2(void) {}
#endif
