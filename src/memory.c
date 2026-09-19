#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include "memory.h"

bool lock_process_memory(void) {
#if defined(MCL_CURRENT) && defined(MCL_FUTURE)
    if (mlockall(MCL_CURRENT | MCL_FUTURE) == 0) {
        return true;
    }
#endif
    return false;
}

void secure_wipe(void *v, size_t n) {
    if (!v || n == 0) {
        return;
    }
#if defined(__GLIBC__) && (__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 25))
    explicit_bzero(v, n);
#else
    volatile unsigned char *p = (volatile unsigned char *)v;
    while (n--) {
        *p++ = 0;
    }
    __asm__ __volatile__("" ::: "memory");
#endif
}
