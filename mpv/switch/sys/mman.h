#ifndef _MEMMAP_H_
#define _MEMMAP_H_

#include <stdio.h>
#include <stdint.h>

#define PROT_READ       0x1
#define PROT_WRITE      0x2
#define MAP_SHARED      0x01
#define MAP_FAILED      ((void *) -1)

#if defined(__SWITCH__)

#define mmap(a, b, c, d, e, f) malloc(b)
#define munmap(a, b) free(a)

#else

void *mmap(void *addr, size_t len, int mmap_prot, int mmap_flags, int fildes, size_t off);

int munmap(void *addr, size_t len);

#endif
#endif
