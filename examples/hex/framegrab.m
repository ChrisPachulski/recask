// Loaded into a process with DYLD_INSERT_LIBRARIES: the first frame each CAMetalLayer presents after FRAMEGRAB_DELAY
// seconds is copied out on its presenting command buffer and written to FRAMEGRAB_DIR/layer_<n>.png. It reads the GPU
// texture, not the window server, so it needs no Screen Recording permission. framebufferOnly is forced off so the
// drawable's texture can be copied.
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <ImageIO/ImageIO.h>
#import <objc/runtime.h>
#include <stdatomic.h>

static double t0, delay;
static atomic_int shots;
static NSMutableSet *seen;
static IMP o_next, o_fbo, o_cbpresent, o_dpresent;

static void write_png(const uint8_t *px, NSUInteger w, NSUInteger h, MTLPixelFormat f, int n) {
    BOOL bgra = f == MTLPixelFormatBGRA8Unorm || f == MTLPixelFormatBGRA8Unorm_sRGB;
    BOOL rgba = f == MTLPixelFormatRGBA8Unorm || f == MTLPixelFormatRGBA8Unorm_sRGB;
    if (!bgra && !rgba) { fprintf(stderr, "framegrab: layer %d: pixel format %lu not written\n", n, (unsigned long)f); return; }
    CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
    CGBitmapInfo bi = bgra ? kCGBitmapByteOrder32Little | kCGImageAlphaPremultipliedFirst
                           : kCGBitmapByteOrder32Big | kCGImageAlphaPremultipliedLast;
    CGContextRef ctx = CGBitmapContextCreate((void *)px, w, h, 8, w * 4, cs, bi);
    CGImageRef img = CGBitmapContextCreateImage(ctx);
    NSString *path = [NSString stringWithFormat:@"%s/layer_%d.png", getenv("FRAMEGRAB_DIR"), n];
    CGImageDestinationRef d = CGImageDestinationCreateWithURL((__bridge CFURLRef)[NSURL fileURLWithPath:path],
                                                              CFSTR("public.png"), 1, NULL);
    CGImageDestinationAddImage(d, img, NULL);
    long drawn = 0;
    for (NSUInteger i = 0; i < w * h * 4; i += 4) drawn += px[i + 3] != 0;
    fprintf(stderr, "framegrab: layer %d %lux%lu -> %s (%s), %ld of %lu pixels drawn\n", n, (unsigned long)w, (unsigned long)h,
            path.UTF8String, CGImageDestinationFinalize(d) ? "written" : "FAILED", drawn, (unsigned long)(w * h));
    CFRelease(d); CGImageRelease(img); CGContextRelease(ctx); CGColorSpaceRelease(cs);
}

static void grab(id<MTLCommandBuffer> cb, id<CAMetalDrawable> dr) {
    if (CFAbsoluteTimeGetCurrent() - t0 < delay || atomic_load(&shots) >= 8) return;
    NSValue *key = [NSValue valueWithPointer:(__bridge void *)dr.layer];
    @synchronized (seen) { if ([seen containsObject:key]) return; [seen addObject:key]; }
    id<MTLTexture> t = dr.texture;
    if (t.framebufferOnly) { fprintf(stderr, "framegrab: a framebufferOnly drawable, skipped\n"); return; }
    NSUInteger w = t.width, h = t.height, bpr = w * 4;
    id<MTLBuffer> buf = [cb.device newBufferWithLength:bpr * h options:MTLResourceStorageModeShared];
    id<MTLBlitCommandEncoder> b = [cb blitCommandEncoder];
    [b copyFromTexture:t sourceSlice:0 sourceLevel:0 sourceOrigin:MTLOriginMake(0, 0, 0) sourceSize:MTLSizeMake(w, h, 1)
              toBuffer:buf destinationOffset:0 destinationBytesPerRow:bpr destinationBytesPerImage:bpr * h];
    [b endEncoding];
    int n = atomic_fetch_add(&shots, 1);
    MTLPixelFormat f = t.pixelFormat;
    [cb addCompletedHandler:^(id<MTLCommandBuffer> c) { write_png((const uint8_t *)buf.contents, w, h, f, n); }];
}

static id my_next(CAMetalLayer *self, SEL s) {
    ((void (*)(id, SEL, BOOL))o_fbo)(self, @selector(setFramebufferOnly:), NO);
    return ((id (*)(id, SEL))o_next)(self, s);
}
static void my_fbo(CAMetalLayer *self, SEL s, BOOL v) { ((void (*)(id, SEL, BOOL))o_fbo)(self, s, NO); }
static void my_cbpresent(id self, SEL s, id dr) {
    if ([dr conformsToProtocol:@protocol(CAMetalDrawable)]) grab(self, dr);
    ((void (*)(id, SEL, id))o_cbpresent)(self, s, dr);
}
static void my_dpresent(id self, SEL s) {                              // presented without a command buffer: copy on one of ours
    if (CFAbsoluteTimeGetCurrent() - t0 >= delay && atomic_load(&shots) < 8) {
        id<MTLCommandBuffer> cb = [[((id<CAMetalDrawable>)self).layer.device newCommandQueue] commandBuffer];
        grab(cb, self); [cb commit]; [cb waitUntilCompleted];
    }
    ((void (*)(id, SEL))o_dpresent)(self, s);
}

__attribute__((constructor)) static void start(void) {
    if (!getenv("FRAMEGRAB_DIR")) return;
    delay = getenv("FRAMEGRAB_DELAY") ? atof(getenv("FRAMEGRAB_DELAY")) : 0;
    t0 = CFAbsoluteTimeGetCurrent(); seen = [NSMutableSet new];
    Class layer = [CAMetalLayer class];
    o_next = method_setImplementation(class_getInstanceMethod(layer, @selector(nextDrawable)), (IMP)my_next);
    o_fbo = method_setImplementation(class_getInstanceMethod(layer, @selector(setFramebufferOnly:)), (IMP)my_fbo);
    id<MTLDevice> dev = MTLCreateSystemDefaultDevice();
    Class cb = object_getClass([[dev newCommandQueue] commandBuffer]);    // the driver's private command buffer class
    o_cbpresent = method_setImplementation(class_getInstanceMethod(cb, @selector(presentDrawable:)), (IMP)my_cbpresent);
    CAMetalLayer *l = [CAMetalLayer layer]; l.device = dev; l.drawableSize = CGSizeMake(8, 8);
    Class drawable = object_getClass([l nextDrawable]);
    o_dpresent = method_setImplementation(class_getInstanceMethod(drawable, @selector(present)), (IMP)my_dpresent);
}
