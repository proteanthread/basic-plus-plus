/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: archive.h
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Defines the public interfaces for packaging and extracting self-contained
 *    Execution Archives (.EXE/.BPE).
 *
 * 2. PORTABILITY CONCERNS:
 *    Standard ANSI C17 compliance. Integrates with the platform-independent
 *    Virtual File System (VFS) to resolve paths uniformly on Windows, Linux,
 *    and BSD.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Validation error codes, specific diagnostic print messages.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Function signatures and core chunk loading contracts.
 * ===================================================================== */

#ifndef BASICPP_ARCHIVE_H
#define BASICPP_ARCHIVE_H

#include "memory.h"
#include "standalone/archive/archive_core.h"

/* bpe_save - Pack a source file and its bytecode cache into an execution archive.
 *
 * Parameters:
 *   src_filename - Path to the input plain text .BAS file.
 *   bpp_filename - Optional path to the input compiled .BPP file (NULL if none).
 *   out_filename - Path to the output packaged .EXE/.BPE archive.
 *
 * Returns 0 on success, -1 on failure.
 */
int bpe_save(const char *src_filename, const char *bpp_filename, const char *out_filename);

/* bpe_load - Load and validate a BPE execution archive.
 *
 * Unpacks chunks, performs VM version safety checks, verifies dependency
 * modules, loads source code into the program store, and caches compiled
 * bytecode if compatible.
 *
 * Parameters:
 *   filename - Path to the .EXE/.BPE file.
 *   prog     - Destination program store for the unpacked source.
 *   rt_ptr   - Pointer to active RuntimeState.
 *
 * Returns 0 on success, -1 on failure.
 */
int bpe_load(const char *filename, ProgramStore *prog, void *rt_ptr);

/* bpe_load_from_offset - Load and validate a BPE execution archive at a specific file offset. */
int bpe_load_from_offset(const char *filename, long offset, ProgramStore *prog, void *rt_ptr);

/* bpe_find_embedded_offset - Scan an executable file for an embedded BPE payload. */
long bpe_find_embedded_offset(const char *exe_path);

#endif // BASICPP_ARCHIVE_H
