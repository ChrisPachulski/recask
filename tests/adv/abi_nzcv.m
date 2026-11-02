/* NZCV, a method that reads the flags at entry reached only through a relative method list: -[Flag flag] tests Z before writing
   it, and its only reference is its IMP's 32-bit offset in Flag's __objc_methlist. _call_flag compares and sends it through
   objc_msgSend, a host import whose site stores NZCV only into flag-reading callees: the image must store NZCV at every call (the
   method is address-taken), or the method reads whatever the CPU struct held in Z. */
#import <Foundation/Foundation.h>
#include <stdio.h>
@interface Flag : NSObject
- (long)flag;
@end
@implementation Flag
- (long)flag {
    long v = 0;
#ifdef __aarch64__
    __asm__ volatile("cset %0, eq" : "=r"(v));                          // Z as the caller left it
#endif
    return v;
}
@end
#ifdef __aarch64__
long call_flag(long, id, SEL);
__asm__(".text\n.p2align 2\n"
        ".globl _call_flag\n"
        "_call_flag:\n"
        "  stp x29, x30, [sp, #-16]!\n"
        "  mov x29, sp\n"
        "  cmp x0, #2\n"
        "  mov x0, x1\n"
        "  mov x1, x2\n"
        "  bl _objc_msgSend\n"
        "  ldp x29, x30, [sp], #16\n"
        "  ret\n");
#else
static long call_flag(long a, id o, SEL s) { (void)o; (void)s; return a == 2; }
#endif
int main(void) {
    @autoreleasepool {
        Flag *f = [Flag new];
        for (long a = 1; a <= 4; a++) printf("nzcv %ld -> %ld\n", a, call_flag(a, f, @selector(flag)));
    }
    return 0;
}
