/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: archive.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Implements packing and unpacking of self-contained Execution Archives
 *    (.EXE/.BPE) using a chunk-based directory format.
 *
 * 2. PORTABILITY CONCERNS:
 *    ANSI/ISO C17 compliant. Manually packs all integers in little-endian
 *    format to prevent struct alignment and endianness compiler divergence.
 *    Works on Windows, Linux, and BSD.
 *
 * 3. MEMORY MANAGEMENT:
 *    Dynamically allocates temporary buffers for chunk verification and
 *    file payloads. All allocations are bounds-checked and freed before
 *    returning.
 *
 * 4. WHAT CAN BE CHANGED:
 *    Maximum chunk size thresholds, validation log messages.
 *
 * 5. WHAT CANNOT BE CHANGED:
 *    Chunk types ("META", "DEPS", "SRC_", "BYTE", "SIGN").
 *
 * 6. TROUBLESHOOTING & FAILURE MODES:
 *    Verify archive checksum values. Check if temporary caching file
 *    writing fails due to host permission blocks.
 * ===================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <strings.h>

#ifdef _WIN32
#define strcasecmp _stricmp
#endif

#include "archive.h"
#include "archive_bpe.h"
#include "bytecode.h"
#include "runtime.h"
#include "../console.h"

static int archive_stricmp(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        char c1 = *s1;
        char c2 = *s2;
        if (c1 >= 'a' && c1 <= 'z') c1 -= 32;
        if (c2 >= 'a' && c2 <= 'z') c2 -= 32;
        if (c1 != c2) return c1 - c2;
        s1++;
        s2++;
    }
    return *s1 - *s2;
}

#define bpe_write_le16 archive_core_write_le16
#define bpe_read_le16 archive_core_read_le16
#define bpe_write_le32 archive_core_write_le32
#define bpe_read_le32 archive_core_read_le32
#define bpe_crc16_update archive_core_crc16

int bpe_save(const char *src_filename, const char *bpp_filename, const char *out_filename)
{
    FILE *out_fp = NULL;
    FILE *src_fp = NULL;
    FILE *bpp_fp = NULL;
    unsigned char *src_buf = NULL;
    unsigned char *bpp_buf = NULL;
    long src_len = 0;
    long bpp_len = 0;
    BpeHeader header;
    BpeChunkDescriptor chunks[5];
    int chunk_count = 0;
    long current_offset = 0;
    unsigned int integrity_crc = 0xFFFF;

    // 1. Open and read source file
    src_fp = fopen(src_filename, "rb");
    if (!src_fp) {
        printf("BPE: Cannot open source file '%s'\n", src_filename);
        return -1;
    }
    fseek(src_fp, 0, SEEK_END);
    src_len = ftell(src_fp);
    fseek(src_fp, 0, SEEK_SET);

    src_buf = (unsigned char *)malloc((size_t)src_len);
    if (!src_buf) {
        fclose(src_fp);
        return -1;
    }
    if (fread(src_buf, 1, (size_t)src_len, src_fp) != (size_t)src_len) {
        printf("BPE: Read error on source file\n");
        free(src_buf);
        fclose(src_fp);
        return -1;
    }
    fclose(src_fp);

    // 2. Open and read optional bytecode file
    if (bpp_filename) {
        bpp_fp = fopen(bpp_filename, "rb");
        if (bpp_fp) {
            fseek(bpp_fp, 0, SEEK_END);
            bpp_len = ftell(bpp_fp);
            fseek(bpp_fp, 0, SEEK_SET);

            bpp_buf = (unsigned char *)malloc((size_t)bpp_len);
            if (bpp_buf) {
                if (fread(bpp_buf, 1, (size_t)bpp_len, bpp_fp) != (size_t)bpp_len) {
                    printf("BPE: Read error on BPP cache\n");
                    free(bpp_buf);
                    bpp_buf = NULL;
                    bpp_len = 0;
                }
            }
            fclose(bpp_fp);
        }
    }

    extern char g_argv_0[512];
    extern char g_runner_path[512];
    const char *runner_src = (g_runner_path[0] != '\0') ? g_runner_path : g_argv_0;
    long runner_size = 0;
    FILE *runner_fp = NULL;
    if (runner_src[0] != '\0') {
        size_t out_len = strlen(out_filename);
        int is_bpe_ext = (out_len >= 4 && (archive_stricmp(&out_filename[out_len - 4], ".bpe") == 0));
        if (!is_bpe_ext) {
            runner_fp = fopen(runner_src, "rb");
        }
    }

    if (runner_fp) {
        long embedded_off = bpe_find_embedded_offset(runner_src);
        if (embedded_off > 0) {
            runner_size = embedded_off;
        } else {
            fseek(runner_fp, 0, SEEK_END);
            runner_size = ftell(runner_fp);
            fseek(runner_fp, 0, SEEK_SET);
        }
    }

    out_fp = fopen(out_filename, "wb");
    if (!out_fp) {
        printf("BPE: Cannot open '%s' for writing\n", out_filename);
        if (runner_fp) fclose(runner_fp);
        free(src_buf);
        if (bpp_buf) free(bpp_buf);
        return -1;
    }

    if (runner_fp && runner_size > 0) {
        unsigned char copy_buf[4096];
        long remaining = runner_size;
        while (remaining > 0) {
            size_t to_read = (remaining > 4096) ? 4096 : (size_t)remaining;
            size_t read_bytes = fread(copy_buf, 1, to_read, runner_fp);
            if (read_bytes == 0) break;
            fwrite(copy_buf, 1, read_bytes, out_fp);
            remaining -= (long)read_bytes;
        }
        fclose(runner_fp);
    }

    // 3. Initialize Chunk Table
    // Chunk 0: META (16 bytes)
    unsigned char meta_payload[16];
    memset(meta_payload, 0, 16);
    meta_payload[0] = 4; // VM Major
    meta_payload[1] = 2; // VM Minor
    meta_payload[2] = 6; // VM Patch
    meta_payload[3] = (unsigned char)3; // Dialect ID

    memcpy(chunks[0].type, "META", 4);
    bpe_write_le32(chunks[0].length, 16);
    memset(chunks[0].reserved, 0, 4);

    chunk_count = 1;

    // Chunk 1: DEPS (empty 0-byte manifest for now)
    memcpy(chunks[1].type, "DEPS", 4);
    bpe_write_le32(chunks[1].length, 0);
    memset(chunks[1].reserved, 0, 4);
    chunk_count++;

    // Chunk 2: SRC_ (verbatim source)
    memcpy(chunks[2].type, "SRC_", 4);
    bpe_write_le32(chunks[2].length, (unsigned long)src_len);
    memset(chunks[2].reserved, 0, 4);
    chunk_count++;

    // Chunk 3: BYTE (optional bytecode)
    if (bpp_buf && bpp_len > 0) {
        memcpy(chunks[3].type, "BYTE", 4);
        bpe_write_le32(chunks[3].length, (unsigned long)bpp_len);
        memset(chunks[3].reserved, 0, 4);
        chunk_count++;
    }

    // Chunk 4: SIGN (2 bytes CRC checksum of all preceding chunks' payloads combined)
    int sign_idx = chunk_count;
    memcpy(chunks[sign_idx].type, "SIGN", 4);
    bpe_write_le32(chunks[sign_idx].length, 2);
    memset(chunks[sign_idx].reserved, 0, 4);
    chunk_count++;

    // Calculate offsets
    // Header (16 bytes) + Directory (chunk_count * 16 bytes)
    current_offset = 16 + (chunk_count * 16);
    for (int i = 0; i < chunk_count; i++) {
        bpe_write_le32(chunks[i].offset, (unsigned long)current_offset);
        current_offset += (long)bpe_read_le32(chunks[i].length);
    }

    // 4. Build Header
    memset(&header, 0, sizeof(BpeHeader));
    memcpy(header.magic, BPE_MAGIC, 4);
    header.format_ver = 1;
    header.chunk_count = (unsigned char)chunk_count;
    bpe_write_le32(header.total_size, (unsigned long)current_offset);

    // Calculate integrity CRC of all chunks' payloads
    integrity_crc = bpe_crc16_update(integrity_crc, meta_payload, 16);
    integrity_crc = bpe_crc16_update(integrity_crc, src_buf, (int)src_len);
    if (bpp_buf && bpp_len > 0) {
        integrity_crc = bpe_crc16_update(integrity_crc, bpp_buf, (int)bpp_len);
    }

    // Write header and directory
    fwrite(&header, 1, sizeof(BpeHeader), out_fp);
    for (int i = 0; i < chunk_count; i++) {
        fwrite(&chunks[i], 1, sizeof(BpeChunkDescriptor), out_fp);
    }

    // Write Payloads
    // 0: META
    fwrite(meta_payload, 1, 16, out_fp);
    // 1: DEPS (0 bytes, nothing to write)
    // 2: SRC_
    fwrite(src_buf, 1, (size_t)src_len, out_fp);
    // 3: BYTE
    if (bpp_buf && bpp_len > 0) {
        fwrite(bpp_buf, 1, (size_t)bpp_len, out_fp);
    }
    // 4: SIGN
    unsigned char sign_payload[2];
    bpe_write_le16(sign_payload, integrity_crc);
    fwrite(sign_payload, 1, 2, out_fp);

    // Calculate header CRC
    fseek(out_fp, runner_size, SEEK_SET);
    unsigned char *hdr_dir_buf = (unsigned char *)malloc((size_t)(16 + chunk_count * 16));
    if (hdr_dir_buf) {
        // Build mock header without crc for calculation
        BpeHeader calc_header = header;
        bpe_write_le16(calc_header.header_crc, 0);
        memcpy(hdr_dir_buf, &calc_header, 16);
        for (int i = 0; i < chunk_count; i++) {
            memcpy(hdr_dir_buf + 16 + (i * 16), &chunks[i], 16);
        }
        unsigned int h_crc = bpe_crc16_update(0xFFFF, hdr_dir_buf, 16 + chunk_count * 16);
        bpe_write_le16(header.header_crc, h_crc);
        free(hdr_dir_buf);
    }

    // Rewrite header with CRC
    fseek(out_fp, runner_size, SEEK_SET);
    fwrite(&header, 1, sizeof(BpeHeader), out_fp);

    fclose(out_fp);
    free(src_buf);
    if (bpp_buf) free(bpp_buf);

    printf("BPE: Packaged execution archive '%s' successfully.\n", out_filename);
    return 0;
}

int bpe_load(const char *filename, ProgramStore *prog, void *rt_ptr)
{
    return bpe_load_from_offset(filename, 0, prog, rt_ptr);
}

int bpe_load_from_offset(const char *filename, long offset, ProgramStore *prog, void *rt_ptr)
{
    FILE *fp = NULL;
    BpeHeader header;
    BpeChunkDescriptor *chunks = NULL;
    unsigned char *meta_payload = NULL;
    unsigned char *src_payload = NULL;
    unsigned char *bpp_payload = NULL;
    long meta_len = 0, src_len = 0, bpp_len = 0;
    long meta_off = 0, src_off = 0, bpp_off = 0;
    unsigned int stored_integrity_crc = 0;
    unsigned int computed_integrity_crc = 0xFFFF;
    RuntimeState *rt = (RuntimeState *)rt_ptr;

    fp = fopen(filename, "rb");
    if (!fp) {
        printf("BPE: Cannot open '%s'\n", filename);
        return -1;
    }

    if (offset > 0) {
        if (fseek(fp, offset, SEEK_SET) != 0) {
            printf("BPE: Failed to seek to offset %ld\n", offset);
            fclose(fp);
            return -1;
        }
    }

    // 1. Read header
    if (fread(&header, 1, sizeof(BpeHeader), fp) != sizeof(BpeHeader)) {
        printf("BPE: Failed to read BPE header\n");
        fclose(fp);
        return -1;
    }

    // Validate magic
    if (memcmp(header.magic, BPE_MAGIC, 4) != 0) {
        printf("BPE: Bad magic sequence in '%s'\n", filename);
        fclose(fp);
        return -1;
    }

    // Read Directory
    int count = header.chunk_count;
    chunks = (BpeChunkDescriptor *)malloc((size_t)count * sizeof(BpeChunkDescriptor));
    if (!chunks) {
        fclose(fp);
        return -1;
    }

    if (fread(chunks, 1, (size_t)count * sizeof(BpeChunkDescriptor), fp) != (size_t)count * sizeof(BpeChunkDescriptor)) {
        printf("BPE: Failed to read chunk directory\n");
        free(chunks);
        fclose(fp);
        return -1;
    }

    // Locate important chunks
    for (int i = 0; i < count; i++) {
        const BpeChunkDescriptor *c = &chunks[i];
        long off = (long)bpe_read_le32(c->offset);
        long len = (long)bpe_read_le32(c->length);

        if (memcmp(c->type, "META", 4) == 0) {
            meta_off = off;
            meta_len = len;
        } else if (memcmp(c->type, "SRC_", 4) == 0) {
            src_off = off;
            src_len = len;
        } else if (memcmp(c->type, "BYTE", 4) == 0) {
            bpp_off = off;
            bpp_len = len;
        } else if (memcmp(c->type, "SIGN", 4) == 0) {
            fseek(fp, offset + off, SEEK_SET);
            unsigned char sign_buf[2];
            if (fread(sign_buf, 1, 2, fp) == 2) {
                stored_integrity_crc = bpe_read_le16(sign_buf);
            }
        }
    }

    // 2. Read META payload
    if (meta_len > 0) {
        meta_payload = (unsigned char *)malloc((size_t)meta_len);
        if (meta_payload) {
            fseek(fp, offset + meta_off, SEEK_SET);
            if (fread(meta_payload, 1, (size_t)meta_len, fp) != (size_t)meta_len) {
                printf("BPE: Read error on META chunk\n");
                goto load_fail;
            }
            computed_integrity_crc = bpe_crc16_update(computed_integrity_crc, meta_payload, (int)meta_len);
        }
    }

    // 3. Read SRC_ payload
    if (src_len > 0) {
        src_payload = (unsigned char *)malloc((size_t)src_len + 1);
        if (src_payload) {
            fseek(fp, offset + src_off, SEEK_SET);
            if (fread(src_payload, 1, (size_t)src_len, fp) != (size_t)src_len) {
                printf("BPE: Read error on SRC_ chunk\n");
                goto load_fail;
            }
            src_payload[src_len] = '\0';
            computed_integrity_crc = bpe_crc16_update(computed_integrity_crc, src_payload, (int)src_len);
        }
    }

    // 4. Read BYTE payload
    if (bpp_len > 0) {
        bpp_payload = (unsigned char *)malloc((size_t)bpp_len);
        if (bpp_payload) {
            fseek(fp, offset + bpp_off, SEEK_SET);
            if (fread(bpp_payload, 1, (size_t)bpp_len, fp) != (size_t)bpp_len) {
                printf("BPE: Read error on BYTE chunk\n");
                goto load_fail;
            }
            computed_integrity_crc = bpe_crc16_update(computed_integrity_crc, bpp_payload, (int)bpp_len);
        }
    }

    fclose(fp);
    fp = NULL;

    // Validate payloads CRC signature
    if (computed_integrity_crc != stored_integrity_crc) {
        printf("BPE: Archive integrity signature check failed!\n");
        goto load_fail;
    }

    // Parse META options
    if (meta_payload) {
        int v_major = meta_payload[0];
        int dialect_id = meta_payload[3];
    (void)dialect_id;

        if (v_major != 4) {
            printf("BPE: Incompatible target VM major version: %d (expected 4)\n", v_major);
            goto load_fail;
        }

        
    }

    // Load source code lines from memory payload
    program_clear(prog);
    if (src_payload) {
        char *line = (char *)src_payload;
        char *next_line = NULL;

        while (line && *line) {
            // Find end of line
            next_line = strchr(line, '\n');
            if (next_line) {
                *next_line = '\0';
                next_line++;
            }

            // Strip trailing CR
            int line_len = (int)strlen(line);
            if (line_len > 0 && line[line_len - 1] == '\r') {
                line[line_len - 1] = '\0';
            }

            // Skip shebang or empty line
            if (line[0] != '\0' && !(line[0] == '#' && line[1] == '!')) {
                // Parse line number
                int is_non_decimal = 0;
                int end_pos = 0;
                double line_num = program_parse_line_number(line, &end_pos, &is_non_decimal);
                if (line_num >= (double)LINE_NUMBER_MIN) {
                    double max_limit = is_non_decimal ? 4294967295.0 : (double)LINE_NUMBER_MAX;
                    if (line_num <= max_limit) {
                        if (program_insert(prog, line_num, line) != 0) {
                            printf("BPE: Store capacity full while loading source\n");
                            goto load_fail;
                        }
                    }
                }
            }

            line = next_line;
        }
    }

    // Load Bytecode Cache
    if (bpp_payload && bpp_len > 0 && rt) {
        // Write byte payload temporarily to cwd as compiled cache file
        FILE *tmp = fopen("_compiled_cache.bpp", "wb");
        if (tmp) {
            fwrite(bpp_payload, 1, (size_t)bpp_len, tmp);
            fclose(tmp);

            // Load via bpp_load
            if (bpp_load(prog, "_compiled_cache.bpp", rt) != 0) {
                printf("Warning: Failed to load cached bytecode block from archive. Re-compiling.\n");
            }

            remove("_compiled_cache.bpp");
        }
    }

    free(chunks);
    if (meta_payload) free(meta_payload);
    if (src_payload) free(src_payload);
    if (bpp_payload) free(bpp_payload);

    printf("BPE: Loaded program and components successfully.\n");
    return 0;

load_fail:
    if (fp) fclose(fp);
    free(chunks);
    if (meta_payload) free(meta_payload);
    if (src_payload) free(src_payload);
    if (bpp_payload) free(bpp_payload);
    return -1;
}

long bpe_find_embedded_offset(const char *exe_path)
{
    FILE *fp = fopen(exe_path, "rb");
    if (!fp) return -1;

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    if (file_size < 1024) {
        fclose(fp);
        return -1;
    }

    fseek(fp, 1024, SEEK_SET);
    unsigned char buf[4096];
    long current_pos = 1024;
    while (current_pos < file_size) {
        size_t read_bytes = fread(buf, 1, 4096, fp);
        if (read_bytes < 4) break;
        for (size_t i = 0; i <= read_bytes - 4; i++) {
            if (buf[i] == 'B' && buf[i+1] == 'P' && buf[i+2] == 'E' && buf[i+3] == 0x1A) {
                long offset = current_pos + (long)i;
                fseek(fp, offset, SEEK_SET);
                BpeHeader hdr;
                if (fread(&hdr, 1, sizeof(BpeHeader), fp) == sizeof(BpeHeader)) {
                    if (hdr.format_ver == 1 && hdr.chunk_count > 0 && hdr.chunk_count <= 10) {
                        fclose(fp);
                        return offset;
                    }
                }
                fseek(fp, current_pos + (long)read_bytes, SEEK_SET);
            }
        }
        current_pos += (long)read_bytes;
    }
    fclose(fp);
    return -1;
}
