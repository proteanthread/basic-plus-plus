 // ---
 // BASIC++ Interpreter - builtins_bio.c
 // ---
 //
 // Block I/O (BIO) function handlers for the built-in function
 // registry. These are the Option B "full primitive set" for
 // block-oriented (random-access) device I/O.
 //
 // BIO functions provide position-addressed read/write access
 // to files and devices. They complement the SIO (stream)
 // functions and the existing FIELD/GET/PUT wrappers (Option A).
 //
 // FUNCTIONS:
 //   BIOREAD$(chan, pos, len)     - Read block at position
 //   BIOWRITE(chan, pos, data$)   - Write block at position
 //   BIOSTATUS(chan)              - Block-level status flags
 //   BIOSIZE(chan)                - Total size of file/device
 //   BIOCHECKSUM(data$)          - CRC-16 checksum of data
 //   BIOCOMPARE(chan, pos, data$) - Compare block with data
 //   BIOFILL(chan, pos, len, val) - Fill block with byte value
 //   BIOCOPY(chan, src, dst, len) - Copy block within file
 //
//
// HOW TO EXTEND:
//   To add a new built-in function:
//   1. Write a handler: BValue my_func(BValue *args, int argc, void *ctx)
//   2. Register it in the init function with funcreg_add().
//   3. Specify min/max argument counts and return type.
//
// TROUBLESHOOTING:
//   - Wrong arg count: check min_args/max_args in registration.
//   - Type mismatch: use bval_to_float/bval_to_int for conversion.
 // ---

#include <stdio.h>
#include <string.h>
#include "builtins.h"
#include "fileio.h"
#include "value.h"
#include "runtime.h"
#include "stringpool.h"
#include "vdev.h"
#include "txn.h"

 // BIOREAD$(chan, pos, len) - Read block at absolute position.
 //
 // Seeks to 'pos', reads 'len' bytes, returns as string.
 // Does not alter the channel's stream position permanently
 // (saves and restores it).
 //
 // Category: FCAT_IO | Safety: FSAFE_IO
BValue builtin_bioread(BValue *args, int argc, void *rt)
{
 RuntimeState *state = (RuntimeState *)rt;
 int chan, len, actual;
 long pos, saved;
 FILE *fp;
 VDev *vd;
 char *poolbuf;
 char tmpbuf[1024];

 (void)argc;
 chan = (int)bval_to_int(&args[0]);
 pos  = (long)bval_to_int(&args[1]);
 len  = (int)bval_to_int(&args[2]);
 if (len <= 0) return bval_string(NULL, 0);
 if (len > 1024) len = 1024;

 // VDev path
 vd = fileio_get_channel_vdev(chan);
 if (vd != NULL) {
  if (vd->dev_seek != NULL && vd->dev_read != NULL) {
   long orig = vd->dev_seek(vd, 0, 1); // SEEK_CUR
   vd->dev_seek(vd, pos, 0); // SEEK_SET
   actual = vd->dev_read(vd, tmpbuf, len);
   if (orig >= 0) vd->dev_seek(vd, orig, 0);
  } else {
   return bval_string(NULL, 0);
  }
  if (actual <= 0) return bval_string(NULL, 0);
  poolbuf = strpool_alloc(&state->strpool, actual);
  if (poolbuf == NULL) return bval_string(NULL, 0);
  memcpy(poolbuf, tmpbuf, (size_t)actual);
  return bval_string(poolbuf, actual);
 }

 // File path
 fp = fileio_get_fp(chan);
 if (fp == NULL) return bval_string(NULL, 0);
 saved = ftell(fp);
 fseek(fp, pos, SEEK_SET);
 actual = (int)fread(tmpbuf, 1, (size_t)len, fp);
 fseek(fp, saved, SEEK_SET); // restore
 if (actual <= 0) return bval_string(NULL, 0);
 poolbuf = strpool_alloc(&state->strpool, actual);
 if (poolbuf == NULL) return bval_string(NULL, 0);
 memcpy(poolbuf, tmpbuf, (size_t)actual);
 return bval_string(poolbuf, actual);
}

 // BIOWRITE(chan, pos, data$) - Write block at position.
 //
 // Seeks to 'pos', writes data$, returns bytes written.
 // Restores original position after write.
 //
 // Category: FCAT_IO | Safety: FSAFE_IO
BValue builtin_biowrite(BValue *args, int argc, void *rt)
{
 int chan, written;
 long pos, saved;
 const char *data;
 int dlen;
 FILE *fp;
 VDev *vd;

 (void)argc; (void)rt;
 chan = (int)bval_to_int(&args[0]);
 pos  = (long)bval_to_int(&args[1]);
 if (!bval_is_string(&args[2]))
  return bval_int(0);
 data = args[2].v.sval.data;
 dlen = args[2].v.sval.length;
 if (data == NULL || dlen <= 0)
  return bval_int(0);

 // VDev path
 vd = fileio_get_channel_vdev(chan);
 if (vd != NULL) {
  if (vd->dev_seek != NULL && vd->dev_write != NULL) {
   long orig = vd->dev_seek(vd, 0, 1);
   vd->dev_seek(vd, pos, 0);
   written = vd->dev_write(vd, data, dlen);
   if (orig >= 0) vd->dev_seek(vd, orig, 0);
  } else {
   return bval_int(0);
  }
  return bval_int((long)written);
 }

 // File path
 fp = fileio_get_fp(chan);
 if (fp == NULL) return bval_int(0);

 // TXN: journal original data before overwriting
 txn_journal_write(chan, pos, dlen, 0);

 saved = ftell(fp);
 fseek(fp, pos, SEEK_SET);
 written = (int)fwrite(data, 1, (size_t)dlen, fp);
 fseek(fp, saved, SEEK_SET);
 return bval_int((long)written);
}

 // BIOSTATUS(chan) - Block-level status.
 //
 // Returns a bitmask:
 //   Bit 0 (1):   Channel open
 //   Bit 1 (2):   Seekable
 //   Bit 2 (4):   Random-access mode
 //   Bit 3 (8):   Binary mode
 //   Bit 4 (16):  Read-capable
 //   Bit 5 (32):  Write-capable
 //   Bit 6 (64):  VDev-backed with binary I/O
 //
 // Category: FCAT_IO | Safety: FSAFE_IO
BValue builtin_biostatus(BValue *args, int argc, void *rt)
{
 int chan;
 int flags = 0;
 FILE *fp;
 VDev *vd;
 int mode;

 (void)argc; (void)rt;
 chan = (int)bval_to_int(&args[0]);

 fp = fileio_get_fp(chan);
 vd = fileio_get_channel_vdev(chan);

 if (fp == NULL && vd == NULL)
  return bval_int(0);

 flags |= 1; // open

 if (fp != NULL) {
  flags |= 2; // seekable (files always are)
  flags |= 16; // read
  flags |= 32; // write
 }

 mode = fileio_get_channel_mode(chan);
 if (mode == 3) flags |= 4; // FCHAN_RANDOM
 if (mode == 4) flags |= 8; // FCHAN_BINARY

 if (vd != NULL) {
  if (vd->dev_seek != NULL) flags |= 2;
  if (vd->dev_read != NULL) flags |= 16 | 64;
  if (vd->dev_write != NULL) flags |= 32 | 64;
 }

 return bval_int((long)flags);
}

 // BIOSIZE(chan) - Total size of file or device.
 //
 // For files: returns file length in bytes.
 // For VDevs: returns -1 (size unknown/not applicable).
 //
 // Category: FCAT_IO | Safety: FSAFE_IO
BValue builtin_biosize(BValue *args, int argc, void *rt)
{
 int chan;
 FILE *fp;
 long cur, size;

 (void)argc; (void)rt;
 chan = (int)bval_to_int(&args[0]);

 fp = fileio_get_fp(chan);
 if (fp == NULL) return bval_int(-1);

 cur = ftell(fp);
 fseek(fp, 0L, SEEK_END);
 size = ftell(fp);
 fseek(fp, cur, SEEK_SET);
 return bval_int(size);
}

 // BIOCHECKSUM(data$) - CRC-16/CCITT checksum.
 //
 // Computes a 16-bit CRC over the string data.
 // Polynomial: 0x1021 (CRC-CCITT).
 // Initial value: 0xFFFF.
 //
 // Useful for verifying block integrity after reads,
 // comparing device data, or building error-detection
 // protocols for USB/serial communication.
 //
 // Category: FCAT_UTIL | Safety: FSAFE_PURE
BValue builtin_biochecksum(BValue *args, int argc, void *rt)
{
 const char *data;
 int dlen, i, j;
 unsigned int crc = 0xFFFF;

 (void)argc; (void)rt;
 if (!bval_is_string(&args[0]))
  return bval_int(0);
 data = args[0].v.sval.data;
 dlen = args[0].v.sval.length;
 if (data == NULL || dlen <= 0)
  return bval_int(0);

 for (i = 0; i < dlen; i++) {
  crc ^= ((unsigned int)(unsigned char)data[i]) << 8;
  for (j = 0; j < 8; j++) {
   if (crc & 0x8000)
    crc = (crc << 1) ^ 0x1021;
   else
    crc = crc << 1;
  }
  crc &= 0xFFFF;
 }

 return bval_int((long)crc);
}

 // BIOCOMPARE(chan, pos, data$) - Compare block with data.
 //
 // Reads 'len(data$)' bytes from position 'pos' on channel
 // and compares with data$. Returns 0 if identical, or the
 // byte offset (1-based) of the first difference.
 // Returns -1 on read error.
 //
 // Category: FCAT_IO | Safety: FSAFE_IO
BValue builtin_biocompare(BValue *args, int argc, void *rt)
{
 int chan, dlen, actual, i;
 long pos, saved;
 const char *data;
 FILE *fp;
 char tmpbuf[1024];

 (void)argc; (void)rt;
 chan = (int)bval_to_int(&args[0]);
 pos  = (long)bval_to_int(&args[1]);
 if (!bval_is_string(&args[2]))
  return bval_int(-1);
 data = args[2].v.sval.data;
 dlen = args[2].v.sval.length;
 if (data == NULL || dlen <= 0)
  return bval_int(0);
 if (dlen > 1024) dlen = 1024;

 fp = fileio_get_fp(chan);
 if (fp == NULL) return bval_int(-1);

 saved = ftell(fp);
 fseek(fp, pos, SEEK_SET);
 actual = (int)fread(tmpbuf, 1, (size_t)dlen, fp);
 fseek(fp, saved, SEEK_SET);

 if (actual < dlen) return bval_int(-1);

 for (i = 0; i < dlen; i++) {
  if (tmpbuf[i] != data[i])
   return bval_int((long)(i + 1));
 }
 return bval_int(0); // identical
}

 // BIOFILL(chan, pos, len, val) - Fill block with byte.
 //
 // Writes 'len' bytes of value 'val' starting at position
 // 'pos'. Returns the number of bytes written.
 // Useful for zeroing sectors or initializing device buffers.
 //
 // Category: FCAT_IO | Safety: FSAFE_IO
BValue builtin_biofill(BValue *args, int argc, void *rt)
{
 int chan, len, val, written;
 long pos, saved;
 FILE *fp;
 char fillbuf[1024];

 (void)argc; (void)rt;
 chan = (int)bval_to_int(&args[0]);
 pos  = (long)bval_to_int(&args[1]);
 len  = (int)bval_to_int(&args[2]);
 val  = (int)bval_to_int(&args[3]);
 if (len <= 0) return bval_int(0);

 memset(fillbuf, val & 0xFF,
        (size_t)(len > 1024 ? 1024 : len));

 fp = fileio_get_fp(chan);
 if (fp == NULL) return bval_int(0);

 // TXN: journal original data before fill
 txn_journal_write(chan, pos, len, 0);

 saved = ftell(fp);
 fseek(fp, pos, SEEK_SET);

 written = 0;
 while (written < len) {
  int chunk = len - written;
  int w;
  if (chunk > 1024) chunk = 1024;
  w = (int)fwrite(fillbuf, 1, (size_t)chunk, fp);
  if (w <= 0) break;
  written += w;
 }

 fseek(fp, saved, SEEK_SET);
 return bval_int((long)written);
}

 // BIOCOPY(chan, src, dst, len) - Copy block within file.
 //
 // Copies 'len' bytes from position 'src' to position 'dst'
 // within the same file channel. Returns bytes copied.
 // Handles overlapping regions correctly.
 //
 // Category: FCAT_IO | Safety: FSAFE_IO
BValue builtin_biocopy(BValue *args, int argc, void *rt)
{
 int chan, len, copied;
 long src, dst, saved;
 FILE *fp;
 char tmpbuf[1024];

 (void)argc; (void)rt;
 chan = (int)bval_to_int(&args[0]);
 src  = (long)bval_to_int(&args[1]);
 dst  = (long)bval_to_int(&args[2]);
 len  = (int)bval_to_int(&args[3]);
 if (len <= 0) return bval_int(0);

 fp = fileio_get_fp(chan);
 if (fp == NULL) return bval_int(0);

 // TXN: journal destination region before copy
 txn_journal_write(chan, dst, len, 0);

 saved = ftell(fp);

 // Handle overlap: if dst > src, copy backwards
 if (dst > src && dst < src + len) {
  // Overlap: reverse copy in chunks
  int offset = len;
  copied = 0;
  while (offset > 0) {
   int chunk = offset > 1024 ? 1024 : offset;
   int actual;
   offset -= chunk;
   fseek(fp, src + offset, SEEK_SET);
   actual = (int)fread(tmpbuf, 1, (size_t)chunk, fp);
   if (actual <= 0) break;
   fseek(fp, dst + offset, SEEK_SET);
   actual = (int)fwrite(tmpbuf, 1, (size_t)actual, fp);
   if (actual <= 0) break;
   copied += actual;
  }
 } else {
  // No overlap or dst < src: forward copy
  int offset = 0;
  copied = 0;
  while (offset < len) {
   int chunk = len - offset;
   int actual;
   if (chunk > 1024) chunk = 1024;
   fseek(fp, src + offset, SEEK_SET);
   actual = (int)fread(tmpbuf, 1, (size_t)chunk, fp);
   if (actual <= 0) break;
   fseek(fp, dst + offset, SEEK_SET);
   actual = (int)fwrite(tmpbuf, 1, (size_t)actual, fp);
   if (actual <= 0) break;
   offset += actual;
   copied += actual;
  }
 }

 fseek(fp, saved, SEEK_SET);
 return bval_int((long)copied);
}
