// longjmp lands in run, the frame that called setjmp (a resume point, so never split), two frames below the call it
// made; run's caller keeps its values in callee-saved registers across the call (recompile abi_check: setjmp's second
// return has the call's sp and x29).
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>

static jmp_buf env;
static volatile long seed = 3;                                         // keeps caller's values out of reach of constant folding

__attribute__((noinline)) static void deep(int n) { if (n == 0) longjmp(env, 7); }
__attribute__((noinline)) static long step(long x, int n) {         // changes the callee-saved registers caller keeps, under its saves
    long k0 = x ^ seed, k1 = x * seed, k2 = x + seed, k3 = x - seed, k4 = seed << (x & 7), k5 = seed >> (x & 3);
    deep(n);
    return k0 + k1 * 3 + k2 * 5 + k3 * 7 + k4 * 11 + k5 * 13;
}

__attribute__((noinline)) static long run(long x, int n) {
    if (setjmp(env)) return -1;
    return step(x, n) + step(x + 1, n - 1);
}

__attribute__((noinline)) static long caller(long a, int n) {
    long v0 = a ^ seed, v1 = a * seed, v2 = a + seed * 5, v3 = a - seed * 7, v4 = seed << a;
    long r = run(a, n);
    return r + v0 * 10000 + v1 * 1000 + v2 * 100 + v3 * 10 + v4;
}

int main(int argc, char **argv) {
    long a = argc + 1;                                                 // 2 when run without arguments
    for (int n = 0; n < 4; n++) printf("sjmp %d -> %ld\n", n, caller(a, n));
    return 0;
}
