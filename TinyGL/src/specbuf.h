// FILENAME: specbuf.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: platform, memory
// Provides core logic and interface definitions for specbuf within BASIC++.
//
// ---- Includes ----

#ifndef _tgl_specbuf_h_
#define _tgl_specbuf_h_

// Max # of specular light pow buffers
#define MAX_SPECULAR_BUFFERS 8
// # of entries in specular buffer
#define SPECULAR_BUFFER_SIZE 1024
// specular buffer granularity
#define SPECULAR_BUFFER_RESOLUTION 1024

typedef struct GLSpecBuf {
  int shininess_i;
  int last_used;
  float buf[SPECULAR_BUFFER_SIZE+1];
  struct GLSpecBuf *next;
} GLSpecBuf;

GLSpecBuf *specbuf_get_buffer(GLContext *c, const int shininess_i, 
                              const float shininess);
void specbuf_cleanup(GLContext *c); // free all memory used

#endif // _tgl_specbuf_h_
