/* #3 loud: an untyped host code pointer (dlsym of a function the guest never imports, by a name built at run time, so no
   string in the image types it from its declaration as fnptr's literal name is) must not be called with a guessed
   signature; the translated program stops at the call. */
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

static volatile char REV[] = "modnars";                       /* "srandom" backwards: never a C string in the image */

int main(void) {
    char name[sizeof REV];
    for (size_t i = 0; i + 1 < sizeof REV; i++) name[i] = REV[sizeof REV - 2 - i];
    name[sizeof REV - 1] = 0;
    void (*f)(unsigned) = (void (*)(unsigned))dlsym(RTLD_DEFAULT, name);
    f(3);
    printf("random %ld\n", random());
    return 0;
}
