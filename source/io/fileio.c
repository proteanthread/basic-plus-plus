/*
 * ---
 * BASIC++ Interpreter - fileio.c
 * ---
 *
 * File I/O implementation for SAVE and LOAD commands.
 *
 * DESIGN RATIONALE:
 * File operations use only ANSI C stdio functions (fopen, fgets,
 * fprintf, fclose) for maximum portability. No POSIX-specific
 * or Windows-specific APIs are used.
 *
 * Files are plain text with one BASIC line per file line. This
 * format is human-readable, editable in any text editor, and
 * compatible across all target platforms.
 *
 * SAVE uses "w" mode (text mode) which handles line-ending
 * conversion automatically on each platform:
 * - Windows: writes \r\n
 * - Linux: writes \n
 * - FreeDOS: writes \r\n
 *
 * LOAD uses fgets() which handles all line-ending formats.
 *
 * ---
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef _WIN32
#include <io.h>
#include <sys/locking.h>
#endif
#include "fileio.h"
#include "errors.h"
#include "config.h"
#include "vdev.h"
#include "vdev_net.h"
#include "device_alias.h"
#include "module.h"

/*
 * fileio_save - Write the program to a text file.
 *
 * Each line is written exactly as stored, followed by a newline.
 * The output file is plain ASCII text.
 */
int fileio_save(ProgramStore *store, const char *filename)
{
 FILE *fp;
 int i;

 fp = fopen(filename, "w");
 if (fp == NULL) {
 error_raise(ERR_HOW, 0);
 return -1;
 }

 for (i = 0; i < store->count; i++) {
 fprintf(fp, "%s\n", store->lines[i].text);
 }

 fclose(fp);
 return 0;
}

/*
 * fileio_load - Read a program from a text file.
 *
 * Reads the file line by line. Each line must start with a line
 * number. Lines are inserted into the program store in order.
 *
 * The caller is responsible for clearing the program store
 * before calling this (for LOAD behavior).
 *
 * Lines without line numbers are silently skipped (they might
 * be comments or blank lines in the file).
 */
int fileio_load(ProgramStore *store, const char *filename)
{
 FILE *fp;
 char buf[INPUT_BUFFER_SIZE];

 fp = fopen(filename, "r");
 if (fp == NULL) {
 error_raise(ERR_HOW, 0);
 return -1;
 }

 while (fgets(buf, INPUT_BUFFER_SIZE, fp) != NULL) {
 int line_num;
 int i;
 int len;
 char *endptr;

 /* Strip trailing newline/carriage return */
 len = (int)strlen(buf);
 while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r')) {
 buf[--len] = '\0';
 }

 /* Skip empty lines */
 if (len == 0) {
 continue;
 }

 /* Skip shebang line (#!/usr/bin/env basicpp) */
 if (buf[0] == '#' && buf[1] == '!') {
 continue;
 }

 /* Skip leading whitespace */
 i = 0;
 while (i < len && (buf[i] == ' ' || buf[i] == '\t')) {
 i++;
 }

 /* Parse line number */
 if (!isdigit((unsigned char)buf[i])) {
 continue; /* skip lines without line numbers */
 }

 line_num = (int)strtol(buf + i, &endptr, 10);
 if (endptr == buf + i) {
 continue; /* no valid number */
 }

 /* Validate line number range */
 if (line_num < LINE_NUMBER_MIN || line_num > LINE_NUMBER_MAX) {
 continue; /* out of range - skip */
 }

 /* Insert the line (using the full text as-is) */
 if (program_insert(store, line_num, buf) != 0) {
 fclose(fp);
 return -1; /* store full - ERR_SORRY already raised */
 }
 }

 fclose(fp);
 return 0;
}

/*
 * fileio_merge - Merge a file into the existing program.
 *
 * Like fileio_load but does NOT clear the current program.
 * Lines from the file overwrite existing lines with the same
 * line number. New lines are inserted in sorted order.
 *
 * This is the standard MERGE behavior in most BASIC dialects.
 */
int fileio_merge(ProgramStore *store, const char *filename)
{
 FILE *fp;
 char buf[INPUT_BUFFER_SIZE];

 fp = fopen(filename, "r");
 if (fp == NULL) {
 error_raise(ERR_HOW, 0);
 return -1;
 }

 while (fgets(buf, INPUT_BUFFER_SIZE, fp) != NULL) {
 int line_num;
 int i;
 int len;
 char *endptr;

 /* Strip trailing newline/carriage return */
 len = (int)strlen(buf);
 while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r')) {
 buf[--len] = '\0';
 }

 if (len == 0) continue;

 /* Skip leading whitespace */
 i = 0;
 while (i < len && (buf[i] == ' ' || buf[i] == '\t')) {
 i++;
 }

 if (!isdigit((unsigned char)buf[i])) continue;

 line_num = (int)strtol(buf + i, &endptr, 10);
 if (endptr == buf + i) continue;
 if (line_num < LINE_NUMBER_MIN || line_num > LINE_NUMBER_MAX) continue;

 if (program_insert(store, line_num, buf) != 0) {
 fclose(fp);
 return -1;
 }
 }

 fclose(fp);
 return 0;
}

/*
 * fileio_chain - Load a program and prepare for execution.
 *
 * Clears the current program, loads the file, and returns 0
 * to signal that the caller should start RUN. The caller
 * (parse_chain_cmd) handles triggering execution.
 */
int fileio_chain(ProgramStore *store, const char *filename)
{
 /* Clear existing program */
 program_clear(store);

 /* Load the new program */
 return fileio_load(store, filename);
}

/* --- File Channel I/O Implementation ---
 *
 * Static file channel table. Channels are 1-based (user-facing),
 * stored 0-based internally. All operations validate channel
 * numbers and raise ERR_HOW for invalid/mismatched operations.
 */

static FileChannel channels[MAX_FILE_CHANNELS];

static void channel_reset(int idx)
{
 channels[idx].fp = NULL;
 channels[idx].vdev = NULL;
 channels[idx].mode = FCHAN_CLOSED;
 channels[idx].record_len = 128;
 channels[idx].field_count = 0;
 channels[idx].current_rec = 0;
 channels[idx].e116_org = FORG_SEQUENTIAL;
 channels[idx].e116_rec = FREC_DISPLAY;
 memset(channels[idx].record_buf, ' ',
 MAX_RECORD_LEN);
}

void fileio_channels_init(void)
{
 int i;
 for (i = 0; i < MAX_FILE_CHANNELS; i++)
 channel_reset(i);
}

void fileio_channels_cleanup(void)
{
 int i;
 for (i = 0; i < MAX_FILE_CHANNELS; i++) {
 if (channels[i].fp != NULL) {
 fclose(channels[i].fp);
 }
 if (channels[i].vdev != NULL &&
 channels[i].vdev->dev_close != NULL) {
 channels[i].vdev->dev_close(channels[i].vdev);
 }
 channel_reset(i);
 }
}

int fileio_open(int chan, const char *filename,
 int mode, int line_num)
{
 const char *fmode;
 int idx = chan - 1;

 if (idx < 0 || idx >= MAX_FILE_CHANNELS) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }

 /* Channel already open? */
 if (channels[idx].fp != NULL || channels[idx].vdev != NULL) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }

 /* Network routing: legacy TCP:/UDP: format */
 if (strncmp(filename, "TCP:", 4) == 0 || strncmp(filename, "UDP:", 4) == 0) {
  VDev *netdev = vdev_net_open(filename);
  if (!netdev) {
   error_raise(ERR_HOW, line_num);
   return -1;
  }
  channels[idx].vdev = netdev;
  channels[idx].mode = FCHAN_DEVICE;
  channels[idx].record_len = 1;
  channels[idx].field_count = 0;
  channels[idx].current_rec = 0;
  return 0;
 }

 /* NET: — Core virtual network (always available).
  *
  * No module activation required. Routes through the
  * core vdev_net layer for basic TCP/UDP/protocol socket
  * access. Supports all registered protocols:
  *   NET:TCP://host:port
  *   NET:HTTP://example.com:80/path
  *   NET:SSH://host:22
  *   etc.
  */
 if ((filename[0] == 'N' || filename[0] == 'n') &&
     (filename[1] == 'E' || filename[1] == 'e') &&
     (filename[2] == 'T' || filename[2] == 't') &&
      filename[3] == ':') {
  VDev *netdev = vdev_net_open(filename);
  if (!netdev) {
   error_raise(ERR_HOW, line_num);
   return -1;
  }
  channels[idx].vdev = netdev;
  channels[idx].mode = FCHAN_DEVICE;
  channels[idx].record_len = 1;
  channels[idx].field_count = 0;
  channels[idx].current_rec = 0;
  return 0;
 }

 /* N: — FujiNet network device (requires MODULE "FUJINET").
  *
  * Routes through the FujiNet VDev for advanced features:
  * JSON parsing, TNFS mounts, HTTP header management,
  * AppKey, Telnet IAC negotiation, etc.
  *
  * Without MODULE "FUJINET", OPEN "N:..." raises an error.
  * Use OPEN "NET:..." for core networking without FujiNet.
  */
 if ((filename[0] == 'N' || filename[0] == 'n') &&
      filename[1] == ':') {
  if (!module_is_active("FUJINET")) {
   printf("Module FUJINET is not active.\n"
          "Use MODULE \"FUJINET\" first, or "
          "use NET: for core networking.\n");
   error_raise(ERR_HOW, line_num);
   return -1;
  }
  {
   int dev_id = vdev_find_by_name("N:");
   if (dev_id >= 0) {
    const char *dev_mode;
    switch (mode) {
    case FCHAN_INPUT:  dev_mode = "r";  break;
    case FCHAN_OUTPUT: dev_mode = "w";  break;
    case FCHAN_APPEND: dev_mode = "a";  break;
    default:           dev_mode = "rw"; break;
    }
    return fileio_open_device(chan, dev_id,
                              dev_mode, filename,
                              line_num);
   }
   error_raise(ERR_HOW, line_num);
   return -1;
  }
 }

 /* Device alias routing:
  * Check if the filename matches a registered device alias
  * (e.g., Atari "E:" -> "CON:", GW-BASIC "SCRN:" -> "CON:").
  * If so, resolve the alias and route through VDev. */
 {
  const DeviceAlias *da;
  da = device_alias_resolve(filename);
  if (da != NULL) {
   /* Check direction compatibility */
   if (mode == FCHAN_INPUT &&
       !(da->direction & DEVALIAS_INPUT)) {
    error_raise(ERR_HOW, line_num);
    return -1;
   }
   if ((mode == FCHAN_OUTPUT || mode == FCHAN_APPEND) &&
       !(da->direction & DEVALIAS_OUTPUT)) {
    error_raise(ERR_HOW, line_num);
    return -1;
   }
   /* Look up the target VDev by name */
   {
    int dev_id;
    const char *dev_mode;
    dev_id = vdev_find_by_name(da->target);
    if (dev_id < 0) {
     /* Target VDev not registered.
      * This means the device (e.g., SER:) hasn't been
      * loaded as a module yet. */
     printf("Device '%s' (alias for '%s') "
            "not available.\n",
            da->alias, da->target);
     error_raise(ERR_HOW, line_num);
     return -1;
    }
    switch (mode) {
    case FCHAN_INPUT:  dev_mode = "r";  break;
    case FCHAN_OUTPUT: dev_mode = "w";  break;
    case FCHAN_APPEND: dev_mode = "a";  break;
    default:           dev_mode = "rw"; break;
    }
    return fileio_open_device(chan, dev_id,
                              dev_mode, filename,
                              line_num);
   }
  }
 }

 /* Also check if the filename itself ends with ':'
  * and matches a registered VDev directly (e.g., "CON:",
  * "ERR:", "GPIO17:"). This allows programs to open
  * VDevs by name without needing an alias. */
 {
  int flen = (int)strlen(filename);
  if (flen > 1 && filename[flen - 1] == ':') {
   int dev_id = vdev_find_by_name(filename);
   if (dev_id >= 0) {
    const char *dev_mode;
    switch (mode) {
    case FCHAN_INPUT:  dev_mode = "r";  break;
    case FCHAN_OUTPUT: dev_mode = "w";  break;
    case FCHAN_APPEND: dev_mode = "a";  break;
    default:           dev_mode = "rw"; break;
    }
    return fileio_open_device(chan, dev_id,
                              dev_mode, filename,
                              line_num);
   }
  }
 }

 switch (mode) {
 case FCHAN_INPUT: fmode = "r"; break;
 case FCHAN_OUTPUT: fmode = "w"; break;
 case FCHAN_APPEND: fmode = "a"; break;
 default:
 error_raise(ERR_HOW, line_num);
 return -1;
 }

 channels[idx].fp = fopen(filename, fmode);
 if (channels[idx].fp == NULL) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }

 channels[idx].mode = mode;
 channels[idx].record_len = 128;
 channels[idx].field_count = 0;
 channels[idx].current_rec = 0;
 return 0;
}

int fileio_close(int chan, int line_num)
{
 int idx = chan - 1;

 if (idx < 0 || idx >= MAX_FILE_CHANNELS) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }

 if (channels[idx].fp == NULL &&
 channels[idx].vdev == NULL) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }

 /* Close file-backed channel */
 if (channels[idx].fp != NULL) {
 fclose(channels[idx].fp);
 }

 /* Close device-backed channel */
 if (channels[idx].vdev != NULL &&
 channels[idx].vdev->dev_close != NULL) {
 channels[idx].vdev->dev_close(channels[idx].vdev);
 }

 channel_reset(idx);
 return 0;
}

int fileio_print(int chan, const char *text, int line_num)
{
 int idx = chan - 1;

 if (idx < 0 || idx >= MAX_FILE_CHANNELS) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }

 /* Device-backed channel */
 if (channels[idx].mode == FCHAN_DEVICE &&
 channels[idx].vdev != NULL) {
 return vdev_puts(channels[idx].vdev, text);
 }

 /* File-backed channel */
 if (channels[idx].fp == NULL ||
 (channels[idx].mode != FCHAN_OUTPUT &&
 channels[idx].mode != FCHAN_APPEND &&
 channels[idx].mode != FCHAN_RANDOM &&
 channels[idx].mode != FCHAN_BINARY)) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }

 fprintf(channels[idx].fp, "%s", text);
 return 0;
}

int fileio_print_newline(int chan, int line_num)
{
 int idx = chan - 1;

 if (idx < 0 || idx >= MAX_FILE_CHANNELS) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }
 if (channels[idx].fp == NULL ||
 (channels[idx].mode != FCHAN_OUTPUT &&
 channels[idx].mode != FCHAN_APPEND &&
 channels[idx].mode != FCHAN_RANDOM &&
 channels[idx].mode != FCHAN_BINARY)) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }

 fprintf(channels[idx].fp, "\n");
 fflush(channels[idx].fp);
 return 0;
}

int fileio_input_line(int chan, char *buf, int max_len, int line_num)
{
 int idx = chan - 1;

 if (idx < 0 || idx >= MAX_FILE_CHANNELS) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }

 /* Device-backed channel */
 if (channels[idx].mode == FCHAN_DEVICE &&
 channels[idx].vdev != NULL) {
 return vdev_gets(channels[idx].vdev, buf, max_len);
 }

 /* File-backed channel */
 if (channels[idx].fp == NULL ||
 channels[idx].mode != FCHAN_INPUT) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }

 if (fgets(buf, max_len, channels[idx].fp) == NULL) {
 buf[0] = '\0';
 return -1; /* EOF or error */
 }

 /* Strip trailing newline */
 {
 int len = (int)strlen(buf);
 while (len > 0 &&
 (buf[len - 1] == '\n' || buf[len - 1] == '\r')) {
 buf[--len] = '\0';
 }
 }

 return 0;
}

int fileio_eof(int chan, int line_num)
{
 int idx = chan - 1;
 int ch;

 if (idx < 0 || idx >= MAX_FILE_CHANNELS) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }
 if (channels[idx].fp == NULL) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }

 /*
 * Peek ahead: try to read one byte. If we get EOF,
 * the channel is at end-of-file. Otherwise push the
 * byte back so the next read sees it.
 *
 * This is the standard portable technique to make
 * EOF() predictive (detect EOF before the next read
 * fails, matching GW-BASIC behavior).
 */
 ch = fgetc(channels[idx].fp);
 if (ch == EOF) {
 return 1; /* at EOF */
 }
 ungetc(ch, channels[idx].fp);
 return 0; /* not at EOF */
}

/*
 * fileio_get_fp - Get the raw FILE* for a channel.
 * Returns NULL if channel is invalid or closed.
 */
FILE *fileio_get_fp(int chan)
{
 int idx = chan - 1;
 if (idx < 0 || idx >= MAX_FILE_CHANNELS)
 return NULL;
 return channels[idx].fp;
}

/* --- Random-Access File Operations ---
 */

int fileio_open_random(int chan, const char *filename,
 int rec_len, int line_num)
{
 int idx = chan - 1;
 FILE *fp;

 if (idx < 0 || idx >= MAX_FILE_CHANNELS) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }
 if (channels[idx].fp != NULL) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }

 /*
 * Random-access: "r+b" if file exists, else create
 * with "w+b" so both read and write work.
 */
 fp = fopen(filename, "r+b");
 if (fp == NULL) {
 fp = fopen(filename, "w+b");
 }
 if (fp == NULL) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }

 channels[idx].fp = fp;
 channels[idx].mode = FCHAN_RANDOM;
 if (rec_len <= 0) rec_len = 128;
 if (rec_len > MAX_RECORD_LEN)
 rec_len = MAX_RECORD_LEN;
 channels[idx].record_len = rec_len;
 channels[idx].field_count = 0;
 channels[idx].current_rec = 1;
 memset(channels[idx].record_buf, ' ',
 MAX_RECORD_LEN);
 return 0;
}

int fileio_open_binary(int chan, const char *filename,
 int line_num)
{
 int idx = chan - 1;
 FILE *fp;

 if (idx < 0 || idx >= MAX_FILE_CHANNELS) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }
 if (channels[idx].fp != NULL) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }

 fp = fopen(filename, "r+b");
 if (fp == NULL)
 fp = fopen(filename, "w+b");
 if (fp == NULL) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }

 channels[idx].fp = fp;
 channels[idx].mode = FCHAN_BINARY;
 channels[idx].record_len = 1;
 channels[idx].field_count = 0;
 channels[idx].current_rec = 1;
 return 0;
}

int fileio_set_field(int chan, FieldMap *flds,
 int count, int line_num)
{
 int idx = chan - 1;
 int i;
 int total = 0;

 if (idx < 0 || idx >= MAX_FILE_CHANNELS) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }
 if (channels[idx].fp == NULL ||
 channels[idx].mode != FCHAN_RANDOM) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }
 if (count > MAX_FIELD_MAPS)
 count = MAX_FIELD_MAPS;

 /* Verify total width <= record length */
 for (i = 0; i < count; i++)
 total += flds[i].width;
 if (total > channels[idx].record_len) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }

 /* Install the mappings */
 channels[idx].field_count = count;
 for (i = 0; i < count; i++)
 channels[idx].fields[i] = flds[i];
 return 0;
}

int fileio_get_record(int chan, long rec, int line_num)
{
 int idx = chan - 1;
 int rlen;
 long offset;

 if (idx < 0 || idx >= MAX_FILE_CHANNELS) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }
 if (channels[idx].fp == NULL ||
 channels[idx].mode != FCHAN_RANDOM) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }

 rlen = channels[idx].record_len;
 if (rec < 1) rec = 1;
 offset = (rec - 1) * (long)rlen;

 /* Clear buffer with spaces */
 memset(channels[idx].record_buf, ' ', (size_t)rlen);

 /* Seek and read */
 fseek(channels[idx].fp, offset, SEEK_SET);
 fread(channels[idx].record_buf, 1,
 (size_t)rlen, channels[idx].fp);
 /* Short read is OK - buffer stays padded with spaces */

 channels[idx].current_rec = rec;
 return 0;
}

int fileio_put_record(int chan, long rec, int line_num)
{
 int idx = chan - 1;
 int rlen;
 long offset;

 if (idx < 0 || idx >= MAX_FILE_CHANNELS) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }
 if (channels[idx].fp == NULL ||
 channels[idx].mode != FCHAN_RANDOM) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }

 rlen = channels[idx].record_len;
 if (rec < 1) rec = 1;
 offset = (rec - 1) * (long)rlen;

 fseek(channels[idx].fp, offset, SEEK_SET);
 if (fwrite(channels[idx].record_buf, 1,
 (size_t)rlen,
 channels[idx].fp) != (size_t)rlen) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }
 fflush(channels[idx].fp);

 channels[idx].current_rec = rec;
 return 0;
}

const char *fileio_get_field_value(int chan,
 const char *name, int name_len, int *out_len)
{
 int idx = chan - 1;
 int i;

 if (idx < 0 || idx >= MAX_FILE_CHANNELS)
 return NULL;

 for (i = 0; i < channels[idx].field_count; i++) {
 FieldMap *fm = &channels[idx].fields[i];
 if (fm->name_len == name_len) {
 int j, match = 1;
 for (j = 0; j < name_len; j++) {
 char a = name[j];
 char b = fm->name[j];
 if (a >= 'a' && a <= 'z')
 a = (char)(a - 32);
 if (b >= 'a' && b <= 'z')
 b = (char)(b - 32);
 if (a != b) {
 match = 0; break;
 }
 }
 if (match) {
 *out_len = fm->width;
 return channels[idx].record_buf
 + fm->offset;
 }
 }
 }
 return NULL;
}

int fileio_set_field_value(int chan,
 const char *name, int name_len,
 const char *data, int data_len,
 int justify, int line_num)
{
 int idx = chan - 1;
 int i;

 if (idx < 0 || idx >= MAX_FILE_CHANNELS) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }

 for (i = 0; i < channels[idx].field_count; i++) {
 FieldMap *fm = &channels[idx].fields[i];
 if (fm->name_len == name_len) {
 int j, match = 1;
 char *dst;
 int w;
 for (j = 0; j < name_len; j++) {
 char a = name[j];
 char b = fm->name[j];
 if (a >= 'a' && a <= 'z')
 a = (char)(a - 32);
 if (b >= 'a' && b <= 'z')
 b = (char)(b - 32);
 if (a != b) {
 match = 0; break;
 }
 }
 if (!match) continue;

 dst = channels[idx].record_buf
 + fm->offset;
 w = fm->width;

 /* Clear with spaces */
 memset(dst, ' ', (size_t)w);

 /* Truncate source if too long */
 if (data_len > w) data_len = w;

 if (justify == 0) {
 /* LSET: left-justify */
 if (data && data_len > 0)
 memcpy(dst, data,
 (size_t)data_len);
 } else {
 /* RSET: right-justify */
 if (data && data_len > 0)
 memcpy(dst + w - data_len,
 data, (size_t)data_len);
 }
 return 0;
 }
 }

 /* Not a field - just do plain assignment */
 (void)line_num;
 return 1; /* 1 = not a field, caller can fallback */
}

/* --- Binary File Operations ---
 */

int fileio_get_binary(int chan, long pos,
 char *buf, int len, int line_num)
{
 int idx = chan - 1;
 int nread;

 if (idx < 0 || idx >= MAX_FILE_CHANNELS) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }
 if (channels[idx].fp == NULL ||
 channels[idx].mode != FCHAN_BINARY) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }

 if (pos >= 1)
 fseek(channels[idx].fp, pos - 1, SEEK_SET);
 nread = (int)fread(buf, 1, (size_t)len,
 channels[idx].fp);
 return nread;
}

int fileio_put_binary(int chan, long pos,
 const char *buf, int len,
 int line_num)
{
 int idx = chan - 1;

 if (idx < 0 || idx >= MAX_FILE_CHANNELS) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }
 if (channels[idx].fp == NULL ||
 channels[idx].mode != FCHAN_BINARY) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }

 if (pos >= 1)
 fseek(channels[idx].fp, pos - 1, SEEK_SET);
 if ((int)fwrite(buf, 1, (size_t)len,
 channels[idx].fp) != len) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }
 fflush(channels[idx].fp);
 return 0;
}

/* --- File Locking ---
 * Uses OS-level file locking:
 * Windows: _locking() from <sys/locking.h>
 * POSIX: not supported (returns success)
 */

int fileio_lock(int chan, long start, long end,
 int line_num)
{
 int idx = chan - 1;
 if (idx < 0 || idx >= MAX_FILE_CHANNELS ||
 channels[idx].fp == NULL) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }
#ifdef _WIN32
 {
 long len = end - start + 1;
 int fd;
 if (len <= 0) len = 1;
 fd = _fileno(channels[idx].fp);
 fseek(channels[idx].fp, start - 1, SEEK_SET);
 if (_locking(fd, _LK_LOCK, len) != 0) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }
 }
#endif
 return 0;
}

int fileio_unlock(int chan, long start, long end,
 int line_num)
{
 int idx = chan - 1;
 if (idx < 0 || idx >= MAX_FILE_CHANNELS ||
 channels[idx].fp == NULL) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }
#ifdef _WIN32
 {
 long len = end - start + 1;
 int fd;
 if (len <= 0) len = 1;
 fd = _fileno(channels[idx].fp);
 fseek(channels[idx].fp, start - 1, SEEK_SET);
 if (_locking(fd, _LK_UNLCK, len) != 0) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }
 }
#endif
 return 0;
}

/*
 * fileio_get_channel_mode - Return the mode of a channel.
 * Returns FCHAN_CLOSED if invalid.
 */
int fileio_get_channel_mode(int chan)
{
 int idx = chan - 1;
 if (idx < 0 || idx >= MAX_FILE_CHANNELS)
 return FCHAN_CLOSED;
 return channels[idx].mode;
}

/* --- Device-Backed Channels ---
 * These functions allow OPEN to route a channel through a VDev
 * instead of a FILE*. PRINT # and INPUT # automatically dispatch
 * to the VDev's dev_puts/dev_gets function pointers.
 */

int fileio_open_device(int chan, int dev_id,
 const char *mode, const char *path,
 int line_num)
{
 int idx = chan - 1;
 VDev *dev;

 if (idx < 0 || idx >= MAX_FILE_CHANNELS) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }

 /* Channel already open? */
 if (channels[idx].fp != NULL ||
 channels[idx].vdev != NULL) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }

 /* Get the device */
 dev = vdev_get(dev_id);
 if (dev == NULL) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }

 /* Call device open if supported */
 if (dev->dev_open != NULL) {
 if (dev->dev_open(dev, path, mode) != 0) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }
 }

 channels[idx].vdev = dev;
 channels[idx].mode = FCHAN_DEVICE;
 channels[idx].fp = NULL;
 channels[idx].record_len = 128;
 channels[idx].field_count = 0;
 channels[idx].current_rec = 0;
 return 0;
}

VDev *fileio_get_channel_vdev(int chan)
{
 int idx = chan - 1;
 if (idx < 0 || idx >= MAX_FILE_CHANNELS)
 return NULL;
 if (channels[idx].mode != FCHAN_DEVICE)
 return NULL;
 return channels[idx].vdev;
}

/* --- ECMA-116 Enhanced Files Module ---
 * These functions implement the ECMA-116 file pointer control,
 * record operations, and file management operations. They work
 * alongside the existing GW-BASIC file I/O without conflicts.
 */

void fileio_set_e116_metadata(int chan, int org, int rec)
{
 int idx = chan - 1;
 if (idx < 0 || idx >= MAX_FILE_CHANNELS)
 return;
 channels[idx].e116_org = org;
 channels[idx].e116_rec = rec;
}

/*
 * fileio_set_pointer - Move the file pointer.
 *
 * pos semantics:
 * 0 = POINTER BEGIN (start of file)
 * -1 = POINTER END (end of file)
 * >0 = specific position (record # for RELATIVE,
 * byte position for STREAM/SEQUENTIAL)
 */
int fileio_set_pointer(int chan, long pos, int line_num)
{
 int idx = chan - 1;

 if (idx < 0 || idx >= MAX_FILE_CHANNELS) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }
 if (channels[idx].fp == NULL) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }

 if (pos == 0) {
 /* POINTER BEGIN */
 fseek(channels[idx].fp, 0, SEEK_SET);
 channels[idx].current_rec = 1;
 } else if (pos == -1) {
 /* POINTER END */
 fseek(channels[idx].fp, 0, SEEK_END);
 if (channels[idx].e116_org == FORG_RELATIVE &&
 channels[idx].record_len > 0) {
 long fsize = ftell(channels[idx].fp);
 channels[idx].current_rec =
 fsize / channels[idx].record_len + 1;
 }
 } else {
 /* Specific position */
 if (channels[idx].e116_org == FORG_RELATIVE) {
 /* Record-based: pos is 1-based record # */
 long offset =
 (pos - 1) * (long)channels[idx].record_len;
 fseek(channels[idx].fp, offset, SEEK_SET);
 channels[idx].current_rec = pos;
 } else {
 /* Byte-based: pos is 1-based byte offset */
 fseek(channels[idx].fp, pos - 1, SEEK_SET);
 channels[idx].current_rec = pos;
 }
 }
 return 0;
}

/*
 * fileio_ask_pointer - Get current file pointer position.
 *
 * Returns record number for RELATIVE files,
 * byte position (1-based) for others.
 */
long fileio_ask_pointer(int chan, int line_num)
{
 int idx = chan - 1;
 long pos;

 if (idx < 0 || idx >= MAX_FILE_CHANNELS) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }
 if (channels[idx].fp == NULL) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }

 pos = ftell(channels[idx].fp);
 if (pos < 0) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }

 if (channels[idx].e116_org == FORG_RELATIVE &&
 channels[idx].record_len > 0) {
 /* Return 1-based record number */
 return pos / channels[idx].record_len + 1;
 }

 /* Return 1-based byte position */
 return pos + 1;
}

/*
 * fileio_ask_filesize - Get file size in bytes.
 *
 * Uses fseek(END) + ftell, then restores position.
 */
long fileio_ask_filesize(int chan, int line_num)
{
 int idx = chan - 1;
 long cur_pos, end_pos;

 if (idx < 0 || idx >= MAX_FILE_CHANNELS) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }
 if (channels[idx].fp == NULL) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }

 /* Save current position */
 cur_pos = ftell(channels[idx].fp);
 if (cur_pos < 0) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }

 /* Seek to end and get position */
 fseek(channels[idx].fp, 0, SEEK_END);
 end_pos = ftell(channels[idx].fp);

 /* Restore position */
 fseek(channels[idx].fp, cur_pos, SEEK_SET);

 return end_pos;
}

/*
 * fileio_erase_channel - Truncate/clear file contents.
 *
 * ECMA-116: ERASE # clears the file while keeping it open.
 * Only valid for writable channels (OUTPUT, APPEND, RANDOM, BINARY).
 * INPUT-only channels cannot be truncated.
 */
int fileio_erase_channel(int chan, int line_num)
{
 int idx = chan - 1;
 int mode;

 if (idx < 0 || idx >= MAX_FILE_CHANNELS) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }
 if (channels[idx].fp == NULL) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }

 mode = channels[idx].mode;

 /* Can't truncate a read-only channel */
 if (mode == FCHAN_INPUT) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }

 /*
 * Truncate approach: seek to beginning, then use
 * platform-specific truncation.
 *
 * Windows: _chsize(_fileno(fp), 0)
 * POSIX: ftruncate(fileno(fp), 0)
 * Fallback: freopen with "w+b"
 */
 fflush(channels[idx].fp);
#ifdef _WIN32
 {
 int fd = _fileno(channels[idx].fp);
 if (_chsize(fd, 0) != 0) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }
 }
#else
 /* freopen fallback for non-Windows */
 channels[idx].fp = freopen(NULL, "w+b",
 channels[idx].fp);
 if (channels[idx].fp == NULL) {
 error_raise(ERR_HOW, line_num);
 channel_reset(idx);
 return -1;
 }
#endif

 fseek(channels[idx].fp, 0, SEEK_SET);
 channels[idx].mode = mode;
 channels[idx].current_rec = 1;
 memset(channels[idx].record_buf, ' ',
 MAX_RECORD_LEN);
 return 0;
}


/*
 * fileio_rewrite_record - Overwrite the current record.
 *
 * Seeks back to the start of the current record and
 * writes over it. Only valid for RELATIVE files.
 */
int fileio_rewrite_record(int chan, const char *data,
 int len, int line_num)
{
 int idx = chan - 1;
 int rlen;
 long offset;
 char buf[MAX_RECORD_LEN];
 int write_len;

 if (idx < 0 || idx >= MAX_FILE_CHANNELS) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }
 if (channels[idx].fp == NULL) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }

 if (channels[idx].e116_org != FORG_RELATIVE) {
 /* REWRITE only for RELATIVE files */
 error_raise(ERR_HOW, line_num);
 return -1;
 }

 rlen = channels[idx].record_len;
 if (channels[idx].current_rec < 1)
 channels[idx].current_rec = 1;
 offset = (channels[idx].current_rec - 1) * (long)rlen;

 memset(buf, ' ', (size_t)rlen);
 write_len = len < rlen ? len : rlen;
 if (data && write_len > 0)
 memcpy(buf, data, (size_t)write_len);

 fseek(channels[idx].fp, offset, SEEK_SET);
 if ((int)fwrite(buf, 1, (size_t)rlen,
 channels[idx].fp) != rlen) {
 error_raise(ERR_HOW, line_num);
 return -1;
 }
 fflush(channels[idx].fp);
 return 0;
}
