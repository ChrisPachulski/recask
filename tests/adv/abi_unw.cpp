// The unwinder restores a landing pad's callee-saved registers from the save slots of the frames it unwinds: mid, a
// one-entry (split) frame between the pad and the throw, saves the registers outer keeps its values in, changes them
// and keeps them live across its calls; outer's pad reads them (recompile abi_check: a landing pad's sp and x29 are the
// call's, so its loads are not reads of save slots).
#include <cstdio>
#include <cstdlib>

__attribute__((noinline)) static void thrower(int n, int i) { if (n == 0) throw 40 + i; }

__attribute__((noinline)) static long mid(long a, int n) {
    long k0 = a * 3 + 1, k1 = a * 5 + 2, k2 = a * 7 + 3, k3 = a * 11 + 4, k4 = a * 13 + 5;
    for (int i = 0; i < 4; i++) {
        thrower(n - i, i);
        k0 ^= k1 + i; k1 += k2 ^ i; k2 ^= k3 + k4; k3 += k0; k4 ^= k1;
    }
    return k0 + k1 + k2 + k3 + k4;
}

__attribute__((noinline)) static long outer(long a, int n) {
    long v0 = a + 7, v1 = a * 2 + 9, v2 = a * 4 + 11, v3 = a * 6 + 13, v4 = a * 8 + 15;
    try {
        return mid(a, n) + v0 + v1 + v2 + v3 + v4;
    } catch (int e) {
        return e + v0 * 10000 + v1 * 1000 + v2 * 100 + v3 * 10 + v4;
    }
}

int main(int argc, char **argv) {
    long a = argc + 1;                                                 // 2 when run without arguments
    for (int n = 0; n < 4; n++) printf("unw %d -> %ld\n", n, outer(a, n));
    return 0;
}
