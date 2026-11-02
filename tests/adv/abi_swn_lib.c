/* abi_swn's guest library, linked against Swift so it runs without the register contract (gdyld abi_ok): _ladd9 returns the x9 it
   was entered with plus one, through an adds whose flags nothing reads (no flag facts). */
#ifdef __aarch64__
__asm__(".text\n.p2align 2\n"
        ".globl _ladd9\n"
        "_ladd9:\n"
        "  adds x0, x9, #1\n"
        "  ret\n");
#else
long ladd9(void) { return 0; }
#endif
