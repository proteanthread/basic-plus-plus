/*
 * ---
 * BASIC++ Interpreter - fileio.h
 * ---
 *
 * File I/O subsystem interface.
 *
 * PURPOSE:
 * Handles SAVE, LOAD, and (future) MERGE and CHAIN commands.
 * All file operations use plain ASCII text format - no binary
 * file formats.
 *
 * FILE FORMAT:
 * Saved files are plain text with one program line per file line.
 * Each line is stored exactly as the user entered it, including
 * the line number prefix. Example:
 *
 * 10 PRINT "HELLO"
 * 20 LET A=5
 * 30 GOTO 10
 *
 * This format can be loaded by any text editor and is compatible
 * with the LOAD command.
 *
 * HOW TO EXTEND:
 * MERGE and CHAIN are stubbed - implement the actual logic in
 * fileio.c when needed. The interface is already defined.
 *
 * ---
 */

#ifndef BASICPP_FILEIO_H
#define BASICPP_FILEIO_H

#include <stdio.h>
#include "memory.h"
#include "vdev.h" /* VDev, VDevClass */

/*
 * fileio_save - Save the program to a file.
 *
 * Writes each stored program line to the file in text format.
 * The file is created or overwritten. Returns 0 on success,
 * -1 on failure (raises ERR_HOW).
 *
 * Parameters:
 * store - pointer to the program store
 * filename - null-terminated filename string
 */
int fileio_save(ProgramStore *store, const char *filename);

/*
 * fileio_load - Load a program from a file.
 *
 * Reads lines from the file and inserts them into the program
 * store. The caller should clear the store first (for LOAD) or
 * leave it intact (for MERGE).
 *
 * Returns 0 on success, -1 on failure (raises ERR_HOW).
 *
 * Parameters:
 * store - pointer to the program store
 * filename - null-terminated filename string
 */
int fileio_load(ProgramStore *store, const char *filename);

/*
 * fileio_merge - Merge a file into the current program.
 *
 * Like LOAD but does not clear the existing program. Lines from
 * the file overwrite existing lines with the same number.
 *
 * STUBBED - not implemented in 
 */
int fileio_merge(ProgramStore *store, const char *filename);

/*
 * fileio_chain - Load and transfer execution.
 *
 * Loads a program and begins execution, optionally preserving
 * variable values.
 *
 * STUBBED - not implemented in 
 */
int fileio_chain(ProgramStore *store, const char *filename);

/* --- File Channel I/O ---
 *
 * SYNTAX (GW-BASIC / QBasic compatible):
 * OPEN "filename" FOR INPUT AS #1
 * OPEN "filename" FOR OUTPUT AS #2
 * OPEN "filename" FOR APPEND AS #3
 * CLOSE #1
 * PRINT #2, expr
 * INPUT #1, var
 *
 * Channels are numbered 1..MAX_FILE_CHANNELS.
 * Each channel holds an open FILE* (or VDev*) and its mode.
 *
 * DESIGN:
 * - Static channel table (no dynamic allocation)
 * - Modes: INPUT, OUTPUT, APPEND, RANDOM, BINARY, DEVICE
 * - FCHAN_DEVICE routes I/O through a VDev instead of FILE*
 * - ANSI C stdio for file-backed channels, VDev for device-backed
 *
 * MAX_FILE_CHANNELS raised from 8 to 16.
 * Channels can now be backed by virtual devices (FCHAN_DEVICE).
 */

/* File channel modes */
#define FCHAN_CLOSED 0
#define FCHAN_INPUT 1
#define FCHAN_OUTPUT 2
#define FCHAN_APPEND 3
#define FCHAN_RANDOM 4
#define FCHAN_BINARY 5
#define FCHAN_DEVICE 6 /* device-backed channel */

/* Maximum record length for RANDOM files (GW-BASIC default=128) */
#define MAX_RECORD_LEN 256

/* Maximum field mappings per channel */
#define MAX_FIELD_MAPS 16

/*
 * FieldMap - Maps a string variable to a slice of
 * the record buffer. Established by FIELD #n.
 *
 * name/name_len identify the variable (A$-Z$ or named).
 * offset is the byte offset within the record buffer.
 * width is the field width in bytes.
 */
typedef struct FieldMap {
 char name[32]; /* variable name (upper-cased) */
 int name_len; /* length of name */
 int offset; /* byte offset in record buffer */
 int width; /* field width in bytes */
} FieldMap;

/* ECMA-116 file organization types */
#define FORG_SEQUENTIAL 0 /* sequential access (default) */
#define FORG_RELATIVE 1 /* record-indexed random access */
#define FORG_STREAM 2 /* byte-stream access */

/* ECMA-116 record types */
#define FREC_DISPLAY 0 /* text/human-readable (default) */
#define FREC_INTERNAL 1 /* binary/machine-format */

/* File channel entry */
typedef struct FileChannel {
 FILE *fp; /* open file pointer, or NULL */
 int mode; /* FCHAN_CLOSED .. FCHAN_DEVICE */
 VDev *vdev; /* device-backed channel (or NULL) */

 /* Random-access support */
 int record_len; /* LEN = n (default 128) */
 char record_buf[MAX_RECORD_LEN]; /* record buffer */
 FieldMap fields[MAX_FIELD_MAPS]; /* field mappings */
 int field_count; /* number of active fields */
 long current_rec; /* current record position (1-based) */

 /* ECMA-116 Enhanced Files metadata */
 int e116_org; /* FORG_* organization */
 int e116_rec; /* FREC_* record type */
} FileChannel;

/*
 * fileio_channels_init - Initialize all file channels to closed.
 */
void fileio_channels_init(void);

/*
 * fileio_channels_cleanup - Close all open file channels.
 * Should be called at program end or on NEW/RUN.
 */
void fileio_channels_cleanup(void);

/*
 * fileio_open - Open a file channel.
 * chan: 1..MAX_FILE_CHANNELS
 * mode: FCHAN_INPUT .. FCHAN_BINARY
 * rec_len: record length for RANDOM (0 = default 128)
 * Returns 0 on success, -1 on error.
 */
int fileio_open(int chan, const char *filename,
 int mode, int line_num);

/*
 * fileio_open_random - Open for RANDOM with record length.
 */
int fileio_open_random(int chan, const char *filename,
 int rec_len, int line_num);

/*
 * fileio_open_binary - Open for BINARY.
 */
int fileio_open_binary(int chan, const char *filename,
 int line_num);

/*
 * fileio_close - Close a file channel.
 * Returns 0 on success, -1 on error.
 */
int fileio_close(int chan, int line_num);

/*
 * fileio_print - Write a string to a file channel.
 * Returns 0 on success, -1 on error.
 */
int fileio_print(int chan, const char *text, int line_num);

/*
 * fileio_print_newline - Write a newline to a file channel.
 */
int fileio_print_newline(int chan, int line_num);

/*
 * fileio_input_line - Read a line from a file channel.
 * Reads up to max_len-1 chars into buf. Returns 0 on
 * success, -1 on error or EOF.
 */
int fileio_input_line(int chan, char *buf,
 int max_len, int line_num);

/*
 * fileio_eof - Check if a file channel is at end-of-file.
 * Returns 1 if EOF, 0 if not, -1 on error.
 */
int fileio_eof(int chan, int line_num);

/*
 * fileio_get_fp - Get the raw FILE* for a channel.
 * Returns NULL if channel is invalid or closed.
 */
FILE *fileio_get_fp(int chan);

/* ---
 * Random-Access File Operations
 * ---*/

/*
 * fileio_set_field - Set field mapping for a channel.
 *
 * Clears existing fields and adds new ones.
 * fields/count are arrays of FieldMap entries.
 * Returns 0 on success, -1 on error.
 */
int fileio_set_field(int chan, FieldMap *flds,
 int count, int line_num);

/*
 * fileio_get_record - Read a record from a random-access
 * file into the channel's record buffer.
 *
 * rec: 1-based record number.
 * Returns 0 on success, -1 on error.
 */
int fileio_get_record(int chan, long rec, int line_num);

/*
 * fileio_put_record - Write the channel's record buffer
 * to a random-access file.
 *
 * rec: 1-based record number.
 * Returns 0 on success, -1 on error.
 */
int fileio_put_record(int chan, long rec, int line_num);

/*
 * fileio_get_field_value - Read a field value from the
 * record buffer. Returns pointer into record_buf and
 * sets *out_len to the field width.
 */
const char *fileio_get_field_value(int chan,
 const char *name, int name_len, int *out_len);

/*
 * fileio_set_field_value - Write a value into a field
 * in the record buffer (with LSET/RSET justification).
 *
 * justify: 0=LSET (left), 1=RSET (right)
 */
int fileio_set_field_value(int chan,
 const char *name, int name_len,
 const char *data, int data_len,
 int justify, int line_num);

/* ---
 * Binary File Operations
 * ---*/

/*
 * fileio_get_binary - Read bytes from a binary file.
 * pos: 1-based byte position.
 * buf: output buffer.
 * len: number of bytes to read.
 * Returns bytes actually read, or -1 on error.
 */
int fileio_get_binary(int chan, long pos,
 char *buf, int len, int line_num);

/*
 * fileio_put_binary - Write bytes to a binary file.
 * pos: 1-based byte position.
 * buf: data to write.
 * len: number of bytes to write.
 * Returns 0 on success, -1 on error.
 */
int fileio_put_binary(int chan, long pos,
 const char *buf, int len,
 int line_num);

/* ---
 * File Locking
 * ---*/

/*
 * fileio_lock - Lock a byte range in a file.
 * start/end are 1-based record numbers for RANDOM,
 * or byte positions for BINARY.
 * Returns 0 on success, -1 on error.
 */
int fileio_lock(int chan, long start, long end,
 int line_num);

/*
 * fileio_unlock - Unlock a byte range in a file.
 * Returns 0 on success, -1 on error.
 */
int fileio_unlock(int chan, long start, long end,
 int line_num);

/*
 * fileio_get_channel_mode - Return the mode of a channel.
 * Returns FCHAN_CLOSED if channel invalid or closed.
 */
int fileio_get_channel_mode(int chan);

/* ---
 * Device-Backed Channels
 * ---*/

/*
 * fileio_open_device - Open a channel backed by a VDev.
 *
 * Routes PRINT # and INPUT # through the VDev's function pointers
 * instead of stdio. The channel operates in FCHAN_DEVICE mode.
 *
 * Parameters:
 * chan - channel number (1..MAX_FILE_CHANNELS)
 * dev_id - VDev slot ID (from vdev_register or vdev_find_by_name)
 * mode - open mode hint for dev_open ("r", "w", "rw")
 * path - device path/parameters (passed to dev_open)
 * line_num - BASIC line number for error reporting
 *
 * Returns 0 on success, -1 on error.
 */
int fileio_open_device(int chan, int dev_id,
 const char *mode, const char *path,
 int line_num);

/*
 * fileio_get_channel_vdev - Get the VDev for a device channel.
 * Returns NULL if channel is not device-backed.
 */
VDev *fileio_get_channel_vdev(int chan);

/* ---
 * ECMA-116 Enhanced Files Module
 * ---*/

/*
 * fileio_set_e116_metadata - Set ECMA-116 org/rec on a channel.
 * Called by parser after a successful ECMA-116 OPEN.
 */
void fileio_set_e116_metadata(int chan, int org, int rec);


/*
 * fileio_set_pointer - Move the file pointer.
 * pos: record number (RELATIVE) or byte position (STREAM).
 * 0 = POINTER BEGIN, -1 = POINTER END.
 * Returns 0 on success, -1 on error.
 */
int fileio_set_pointer(int chan, long pos, int line_num);

/*
 * fileio_ask_pointer - Get current file pointer position.
 * Returns record number (RELATIVE) or byte position.
 * Returns -1 on error.
 */
long fileio_ask_pointer(int chan, int line_num);

/*
 * fileio_ask_filesize - Get file size.
 * Returns byte count, or -1 on error.
 */
long fileio_ask_filesize(int chan, int line_num);

/*
 * fileio_erase_channel - Truncate/clear file contents.
 * Closes and reopens the file as empty.
 * Returns 0 on success, -1 on error.
 */
int fileio_erase_channel(int chan, int line_num);

/*
 * fileio_rewrite_record - Overwrite the current record.
 * Seeks back to the start of the current record and
 * rewrites it. Only valid for RELATIVE files.
 * Returns 0 on success, -1 on error.
 */
int fileio_rewrite_record(int chan, const char *data,
 int len, int line_num);

#endif /* BASICPP_FILEIO_H */
