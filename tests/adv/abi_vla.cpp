// A landing pad in a function whose sp moves between its calls (a variable-size stack allocation): the calls disagree
// on sp, so the pad's sp offset stays unknown (recompile path_values), while its x29 offset is the frame's at every call.
#include <alloca.h>
#include <cstdio>

static volatile long seed = 3;                                         // keeps outer's values out of reach of constant folding

__attribute__((noinline)) static void thrower(int n, int i) { if (n == i) throw 40 + i; }
__attribute__((noinline)) static long use(long *p, int m) { long s = 0; for (int i = 0; i < m; i++) s += p[i] * (i + 1); return s; }

__attribute__((noinline)) static long outer(long a, int n) {
    long v0 = a ^ seed, v1 = a * seed, v2 = a + seed * 5, v3 = a - seed * 7, v4 = seed << a;
    try {
        thrower(n, 0);
        long *p = (long *)alloca(sizeof(long) * (n + 2));
        for (int i = 0; i < n + 2; i++) p[i] = a + i;
        thrower(n, 1);
        return use(p, n + 2) + v0 + v1 + v2 + v3 + v4;
    } catch (int e) {
        return e + v0 * 10000 + v1 * 1000 + v2 * 100 + v3 * 10 + v4;
    }
}

int main(int argc, char **argv) {
    long a = argc + 1;                                                 // 2 when run without arguments
    for (int n = 0; n < 3; n++) printf("vla %d -> %ld\n", n, outer(a, n));
    return 0;
}
