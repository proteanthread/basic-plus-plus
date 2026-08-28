// FILENAME: error.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (zgl.h)
// Provides core logic and interface definitions for error within BASIC++.
//
// ---- Includes ----

#include <stdarg.h>
#include "zgl.h"

void gl_fatal_error(char *format, ...)
{
  va_list ap;

  va_start(ap,format);

  fprintf(stderr,"TinyGL: fatal error: ");
  vfprintf(stderr,format,ap);
  fprintf(stderr,"\n");
  exit(1);

  va_end(ap);
}
