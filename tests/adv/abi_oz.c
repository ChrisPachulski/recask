/* Register contract, -Oz argument setup: built with -Oz, every gN becomes `bl _OUTLINED_FUNCTION_k; bl _fN` (no tail
   call, which would keep lr in a register the caller reads after), and the machine outliner's body sets x2 and x3 for
   fN past its own results, then returns. A return must store the caller-saved registers it leaves changed, since
   the next call reads them from the CPU struct. */
#include <stdio.h>
struct S { long a, b, c, d; };
#define F(n) __attribute__((noinline)) long f##n(long a, long b, long c, long d) { return a * 1000 + b * 100 + c * 10 + d + n; }
#define G(n) __attribute__((noinline)) long g##n(struct S *p) { p->d += 1; return f##n(p->a + 1, p->b * 3, p->c ^ 5, p->d) + n + 1; }
F(0) F(1) F(2) F(3) F(4) F(5)
G(0) G(1) G(2) G(3) G(4) G(5)
int main(void) {
    for (long k = 1; k <= 3; k++) {
        struct S s = {k, 2 * k, 3 * k, 4 * k};
        printf("oz %ld: %ld %ld %ld %ld %ld %ld\n", k, g0(&s), g1(&s), g2(&s), g3(&s), g4(&s), g5(&s));
    }
    return 0;
}
