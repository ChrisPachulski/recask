// setjmp called through a pointer: its return site is no resume point (gdyld resume_points takes bl to a setjmp stub),
// so the frame that called it may be split and a longjmp back finds its registers where the capture left them (LIMIT).
#include <setjmp.h>
#include <stdio.h>

static jmp_buf env;
static volatile long seed = 3;
int (*volatile sj)(jmp_buf) = setjmp;                                  // volatile: clang cannot turn the call into bl _setjmp

__attribute__((noinline)) static void deep(int n) { if (n == 0) longjmp(env, 7); }

__attribute__((noinline)) static long run(long x, int n) {
    long k0 = x ^ seed, k1 = x * seed, k2 = x + seed * 5;
    if (sj(env)) return k0 * 100 + k1 * 10 + k2;
    deep(n);
    return k0 + k1 + k2 + n;
}

int main(int argc, char **argv) {
    long a = argc + 1;                                                 // 2 when run without arguments
    for (int n = 0; n < 2; n++) printf("sjp %d -> %ld\n", n, run(a, n));
    return 0;
}
