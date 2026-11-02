/* Register contract, a method outside it reached only through a relative method list: -[Peek peek] reads x19 at entry, a register
   no call passes, and its only reference is its IMP's 32-bit offset in Peek's __objc_methlist. _call_peek sets x19 and sends it
   through objc_msgSend, a host import whose site stores only arguments: the image must keep full register sync (the method is
   address-taken), or the method reads whatever the CPU struct held in x19. */
#import <Foundation/Foundation.h>
#include <stdio.h>
@interface Peek : NSObject
- (long)peek;
@end
@implementation Peek
- (long)peek {
    long v = 0;
#ifdef __aarch64__
    __asm__ volatile("mov %0, x19" : "=r"(v));                          // x19 as the caller left it: no call passes it
#endif
    return v;
}
@end
#ifdef __aarch64__
long call_peek(long, id, SEL);
__asm__(".text\n.p2align 2\n"
        ".globl _call_peek\n"
        "_call_peek:\n"
        "  stp x19, x30, [sp, #-16]!\n"
        "  add x19, x0, #7\n"
        "  mov x0, x1\n"
        "  mov x1, x2\n"
        "  bl _objc_msgSend\n"
        "  ldp x19, x30, [sp], #16\n"
        "  ret\n");
#else
static long call_peek(long a, id o, SEL s) { (void)o; (void)s; return a + 7; }
#endif
int main(void) {
    @autoreleasepool {
        Peek *p = [Peek new];
        for (long a = 1; a <= 4; a++) printf("objc %ld -> %ld\n", a, call_peek(a, p, @selector(peek)));
    }
    return 0;
}
