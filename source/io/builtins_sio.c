/*
 * ---
 * BASIC++ Interpreter - builtins_sio.c
 * ---
 *
 * Stream I/O (SIO) function handlers for the built-in function
 * registry. These are the Option B "full primitive set" for
 * stream-oriented device I/O.
 *
 * SIO functions operate on open file channels (#1-#8) and VDev
 * devices. They provide a lower-level, more uniform interface
 * than the traditional PRINT#/INPUT# wrappers (Option A).
 *
 * Both approaches (Option A wrapper and Option B primitives)
 * work simultaneously on the same channels.
 *
 * FUNCTIONS:
 *   SIOREAD$(chan, n)     - Read n bytes, return as string
 *   SIOREADLN$(chan)      - Read one line, return as string
 *   SIOWRITE(chan, data$) - Write string to channel, return count
 *   SIOSEEK(chan, pos)    - Seek to position, return new pos
 *   SIOFLUSH(chan)        - Flush channel buffers
 *   SIOSTATUS(chan)       - Channel status flags
 *   SIOAVAIL(chan)        - Bytes available for read
 *
 * ---
 */

#include <stdio.h>
#include <string.h>
#include "builtins.h"
#include "fileio.h"
#include "value.h"
#include "runtime.h"
#include "stringpool.h"
#include "vdev.h"
#include "txn.h"

/*
 * SIOREAD$(chan, n) - Read n bytes from channel.
 *
 * Returns a string of up to n bytes read from the channel.
 * Works with both file-backed and VDev-backed channels.
 * Returns empty string on error or EOF.
 *
 * Category: FCAT_IO | Safety: FSAFE_IO
 */
BValue builtin_sioread(BValue *args, int argc, void *rt)
{
 RuntimeState *state = (RuntimeState *)rt;
 int chan, nbytes, actual;
 FILE *fp;
 VDev *vd;
 char *poolbuf;
 char tmpbuf[1024];

 (void)argc;
 chan = (int)bval_to_int(&args[0]);
 nbytes = (int)bval_to_int(&args[1]);
 if (nbytes <= 0) return bval_string(NULL, 0);
 if (nbytes > 1024) nbytes = 1024;

 /* Try VDev-backed channel first */
 vd = fileio_get_channel_vdev(chan);
 if (vd != NULL) {
  if (vd->dev_read != NULL) {
   actual = vd->dev_read(vd, tmpbuf, nbytes);
  } else if (vd->dev_getc != NULL) {
   /* Byte-by-byte fallback */
   int i;
   actual = 0;
   for (i = 0; i < nbytes; i++) {
    int ch = vd->dev_getc(vd);
    if (ch < 0) break;
    tmpbuf[i] = (char)ch;
    actual++;
   }
  } else {
   return bval_string(NULL, 0);
  }
  if (actual <= 0) return bval_string(NULL, 0);
  poolbuf = strpool_alloc(&state->strpool, actual);
  if (poolbuf == NULL) return bval_string(NULL, 0);
  memcpy(poolbuf, tmpbuf, (size_t)actual);
  return bval_string(poolbuf, actual);
 }

 /* File-backed channel */
 fp = fileio_get_fp(chan);
 if (fp == NULL) return bval_string(NULL, 0);
 actual = (int)fread(tmpbuf, 1, (size_t)nbytes, fp);
 if (actual <= 0) return bval_string(NULL, 0);
 poolbuf = strpool_alloc(&state->strpool, actual);
 if (poolbuf == NULL) return bval_string(NULL, 0);
 memcpy(poolbuf, tmpbuf, (size_t)actual);
 return bval_string(poolbuf, actual);
}

/*
 * SIOREADLN$(chan) - Read one line from channel.
 *
 * Reads until newline, CR, or EOF. The newline is consumed
 * but not included in the returned string.
 * Returns empty string on error or EOF.
 *
 * Category: FCAT_IO | Safety: FSAFE_IO
 */
BValue builtin_sioreadln(BValue *args, int argc, void *rt)
{
 RuntimeState *state = (RuntimeState *)rt;
 int chan, len;
 FILE *fp;
 VDev *vd;
 char *poolbuf;
 char tmpbuf[1024];

 (void)argc;
 chan = (int)bval_to_int(&args[0]);

 /* Try VDev-backed channel first */
 vd = fileio_get_channel_vdev(chan);
 if (vd != NULL) {
  if (vd->dev_gets != NULL) {
   if (vd->dev_gets(vd, tmpbuf, 1024) < 0)
    return bval_string(NULL, 0);
  } else if (vd->dev_getc != NULL) {
   int i = 0;
   while (i < 1023) {
    int ch = vd->dev_getc(vd);
    if (ch < 0 || ch == '\n') break;
    if (ch == '\r') continue;
    tmpbuf[i++] = (char)ch;
   }
   tmpbuf[i] = '\0';
  } else {
   return bval_string(NULL, 0);
  }
 } else {
  /* File-backed */
  fp = fileio_get_fp(chan);
  if (fp == NULL) return bval_string(NULL, 0);
  if (fgets(tmpbuf, 1024, fp) == NULL)
   return bval_string(NULL, 0);
 }

 /* Strip trailing newline/CR */
 len = (int)strlen(tmpbuf);
 while (len > 0 && (tmpbuf[len-1] == '\n' ||
                     tmpbuf[len-1] == '\r'))
  len--;
 if (len <= 0) return bval_string(NULL, 0);
 poolbuf = strpool_alloc(&state->strpool, len);
 if (poolbuf == NULL) return bval_string(NULL, 0);
 memcpy(poolbuf, tmpbuf, (size_t)len);
 return bval_string(poolbuf, len);
}

/*
 * SIOWRITE(chan, data$) - Write string to channel.
 *
 * Writes the string contents to the channel. Returns the
 * number of bytes actually written, or 0 on error.
 *
 * Category: FCAT_IO | Safety: FSAFE_IO
 */
BValue builtin_siowrite(BValue *args, int argc, void *rt)
{
 int chan, written;
 const char *data;
 int dlen;
 FILE *fp;
 VDev *vd;

 (void)argc; (void)rt;
 chan = (int)bval_to_int(&args[0]);
 if (!bval_is_string(&args[1]))
  return bval_int(0);
 data = args[1].v.sval.data;
 dlen = args[1].v.sval.length;
 if (data == NULL || dlen <= 0) return bval_int(0);

 /* Try VDev-backed channel */
 vd = fileio_get_channel_vdev(chan);
 if (vd != NULL) {
  if (vd->dev_write != NULL) {
   written = vd->dev_write(vd, data, dlen);
  } else if (vd->dev_puts != NULL) {
   /* String write fallback (null-terminate) */
   char tmpbuf[1024];
   int sl = dlen > 1023 ? 1023 : dlen;
   memcpy(tmpbuf, data, (size_t)sl);
   tmpbuf[sl] = '\0';
   written = (vd->dev_puts(vd, tmpbuf) == 0)
             ? sl : 0;
  } else {
   return bval_int(0);
  }
  return bval_int((long)written);
 }

 /* File-backed */
 fp = fileio_get_fp(chan);
 if (fp == NULL) return bval_int(0);

 /* TXN: journal original data before write */
 {
  long wpos = ftell(fp);
  if (wpos >= 0)
   txn_journal_write(chan, wpos, dlen, 0);
 }

 written = (int)fwrite(data, 1, (size_t)dlen, fp);
 return bval_int((long)written);
}

/*
 * SIOSEEK(chan, pos) - Seek to absolute position.
 *
 * Moves the channel read/write pointer to the given byte
 * position. Returns the new position, or -1 on error.
 * Position 0 = beginning of file/stream.
 *
 * Category: FCAT_IO | Safety: FSAFE_IO
 */
BValue builtin_sioseek(BValue *args, int argc, void *rt)
{
 int chan;
 long pos;
 FILE *fp;
 VDev *vd;

 (void)argc; (void)rt;
 chan = (int)bval_to_int(&args[0]);
 pos = (long)bval_to_int(&args[1]);

 /* Try VDev-backed channel */
 vd = fileio_get_channel_vdev(chan);
 if (vd != NULL) {
  if (vd->dev_seek != NULL) {
   long result = vd->dev_seek(vd, pos, 0); /* SEEK_SET */
   return bval_int(result);
  }
  return bval_int(-1);
 }

 /* File-backed */
 fp = fileio_get_fp(chan);
 if (fp == NULL) return bval_int(-1);
 if (fseek(fp, pos, SEEK_SET) != 0)
  return bval_int(-1);
 return bval_int(ftell(fp));
}

/*
 * SIOFLUSH(chan) - Flush channel buffers.
 *
 * Forces any buffered output to be written to the device.
 * Returns 0 on success, -1 on error.
 *
 * Category: FCAT_IO | Safety: FSAFE_IO
 */
BValue builtin_sioflush(BValue *args, int argc, void *rt)
{
 int chan;
 FILE *fp;
 VDev *vd;

 (void)argc; (void)rt;
 chan = (int)bval_to_int(&args[0]);

 vd = fileio_get_channel_vdev(chan);
 if (vd != NULL) {
  if (vd->dev_flush != NULL)
   return bval_int((long)vd->dev_flush(vd));
  return bval_int(0);
 }

 fp = fileio_get_fp(chan);
 if (fp == NULL) return bval_int(-1);
 fflush(fp);
 return bval_int(0);
}

/*
 * SIOSTATUS(chan) - Channel status flags.
 *
 * Returns a bitmask of channel status:
 *   Bit 0 (1): Channel is open
 *   Bit 1 (2): Channel is at EOF
 *   Bit 2 (4): Channel is file-backed
 *   Bit 3 (8): Channel is VDev-backed
 *   Bit 4 (16): Channel supports read
 *   Bit 5 (32): Channel supports write
 *
 * Returns 0 if channel is not open (no bits set).
 *
 * Category: FCAT_IO | Safety: FSAFE_IO
 */
BValue builtin_siostatus(BValue *args, int argc, void *rt)
{
 int chan;
 int flags = 0;
 FILE *fp;
 VDev *vd;

 (void)argc; (void)rt;
 chan = (int)bval_to_int(&args[0]);

 vd = fileio_get_channel_vdev(chan);
 fp = fileio_get_fp(chan);

 if (fp == NULL && vd == NULL)
  return bval_int(0); /* not open */

 flags |= 1; /* open */

 if (fp != NULL) {
  flags |= 4; /* file-backed */
  if (feof(fp)) flags |= 2;
  flags |= 16; /* assume read */
  flags |= 32; /* assume write */
 }

 if (vd != NULL) {
  flags |= 8; /* VDev-backed */
  if (vd->dev_getc != NULL || vd->dev_gets != NULL ||
      vd->dev_read != NULL)
   flags |= 16; /* read capable */
  if (vd->dev_putc != NULL || vd->dev_puts != NULL ||
      vd->dev_write != NULL)
   flags |= 32; /* write capable */
 }

 return bval_int((long)flags);
}

/*
 * SIOAVAIL(chan) - Bytes available for read.
 *
 * For file-backed channels, returns bytes remaining
 * (file_size - current_position). For VDev channels,
 * uses dev_poll() if available, otherwise returns -1
 * (unknown).
 *
 * Category: FCAT_IO | Safety: FSAFE_IO
 */
BValue builtin_sioavail(BValue *args, int argc, void *rt)
{
 int chan;
 FILE *fp;
 VDev *vd;

 (void)argc; (void)rt;
 chan = (int)bval_to_int(&args[0]);

 vd = fileio_get_channel_vdev(chan);
 if (vd != NULL) {
  if (vd->dev_poll != NULL)
   return bval_int((long)vd->dev_poll(vd));
  return bval_int(-1); /* unknown */
 }

 fp = fileio_get_fp(chan);
 if (fp == NULL) return bval_int(-1);
 {
  long cur = ftell(fp);
  long end;
  if (cur < 0) return bval_int(-1);
  fseek(fp, 0L, SEEK_END);
  end = ftell(fp);
  fseek(fp, cur, SEEK_SET);
  return bval_int(end - cur);
 }
}
