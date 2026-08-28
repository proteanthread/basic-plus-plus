// FILENAME: msghandling.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (light.c, misc.c, specbuf.c)
// NEEDS: platform, memory
// Provides core logic and interface definitions for msghandling within BASIC++.
//
// ---- Includes ----

#ifndef _msghandling_h_
#define _msghandling_h_

extern void tgl_warning(const char *text, ...);
extern void tgl_trace(const char *text, ...);
extern void tgl_fixme(const char *text, ...);

#endif // _msghandling_h_
