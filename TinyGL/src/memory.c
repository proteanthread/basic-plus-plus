// FILENAME: memory.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot, libcore, libengine, libkernel, libserver
// NEEDS: libcore (zgl.h)
// Provides core logic and interface definitions for memory within BASIC++.
//
// ---- Includes ----

#include "zgl.h"

// modify these functions so that they suit your needs

void gl_free(void *p)
{
    free(p);
}

void *gl_malloc(int size)
{
    return calloc(1, size);
}

void *gl_zalloc(int size)
{
    return calloc(1, size);
}
