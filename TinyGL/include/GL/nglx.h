// FILENAME: nglx.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (nanox.c, nglx.c)
// NEEDS: libcore (gl.h)
// Provides core logic and interface definitions for nglx within BASIC++.
//
// ---- Includes ----

#ifndef NGLX_H
#define NGLX_H

#include <microwin/nano-X.h>
#include <GL/gl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void *NGLXContext;
typedef GR_DRAW_ID NGLXDrawable;

extern NGLXContext nglXCreateContext( NGLXContext shareList, int flags );

extern void nglXDestroyContext( NGLXContext ctx );

extern int nglXMakeCurrent( NGLXDrawable drawable,
			    NGLXContext ctx);

extern void nglXSwapBuffers( NGLXDrawable drawable );

#ifdef __cplusplus
}
#endif

#endif
