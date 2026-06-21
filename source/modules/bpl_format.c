/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: bpl_format.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    C-level modular expansions providing hardware wrappers and runtime libraries.
 *
 * 2. WHAT TO EXPECT:
 *    Modules register customized functions at boot to dynamically extend vocabulary.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Module naming, registered commands list, setup/shutdown details.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Module lifecycle dispatcher, keyword override bindings.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Ensure mod_init does not fail. Verify that linkage matches build profiles.
 * ===================================================================== */

 // ---
 // BASIC++ Interpreter - bpl_format.c
 // ---
 //
 // .BPL (BASIC++ Portable Library) serializer/deserializer.
 //
 // IMPLEMENTATION:
 // Binary I/O uses unsigned char buffers with manual byte
 // packing (little-endian) to avoid struct alignment issues.
 // No platform-specific types or endianness assumptions.
 //
 // FORMAT v2: Stores source lines instead of PCode instructions.
 // Each source line is stored as: LE16(vline) + LE16(len) + text.
 // This makes .BPL files portable and OS-independent while
 // preserving the "compile-on-load" interpreter-mode approach.
 //
 // CRC-16/CCITT is used for integrity checking.
 //
 // C89/C90 COMPLIANT.
 //
//
// HOW TO EXTEND:
//   To add new functions to this module:
//   1. Add the function implementation in this file.
//   2. Register it in the module's init function using
//      module_register_function().
//   3. Update the module's header with the new declaration.
//
// TROUBLESHOOTING:
//   - Module not loading: check module_init() registration.
//   - Function not found: verify registration name matches
//     the BASIC keyword exactly (case-insensitive).
 // ---

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bpl_format.h"

// --- Little-endian helpers ---
static void bpl_write_le16(unsigned char *buf, unsigned int val)
{
    buf[0] = (unsigned char)(val & 0xFF);
    buf[1] = (unsigned char)((val >> 8) & 0xFF);
}

static unsigned int bpl_read_le16(const unsigned char *buf)
{
    return (unsigned int)buf[0] |
           ((unsigned int)buf[1] << 8);
}

// --- CRC-16/CCITT ---

// bpl_crc16_update - Continue CRC from a previous seed.
 // Use seed=0xFFFF for the first call, then chain
 // by passing the return value as seed for subsequent calls.
static unsigned int bpl_crc16_update(unsigned int seed,
                                     const unsigned char *data,
                                     int len)
{
    unsigned int crc = seed;
    int j;
    for (int i = 0; i < len; i++) {
        crc ^= (unsigned int)data[i] << 8;
        for (int j = 0; j < 8; j++) {
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x1021;
            else
                crc = crc << 1;
            crc &= 0xFFFF;
        }
    }
    return crc;
}

// bpl_crc16 - Compute CRC-16/CCITT over a single buffer.
unsigned int bpl_crc16(const unsigned char *data, int len)
{
    return bpl_crc16_update(0xFFFF, data, len);
}

// --- bpl_save ---
int bpl_save(const LoadedLibrary *lib, const char *filename)
{
    int i;
    FILE *fp;
    unsigned char header[BPL_HEADER_SIZE];
    unsigned int crc = 0;

    if (!lib || !lib->compiled) {
        printf("Library not compiled, cannot save .BPL.\n");
        return -1;
    }

    fp = fopen(filename, "wb");
    if (!fp) {
        printf("Cannot open '%s' for writing.\n", filename);
        return -1;
    }

    // Build header
    memset(header, 0, BPL_HEADER_SIZE);
    header[0] = (unsigned char)BPL_MAGIC_0;
    header[1] = (unsigned char)BPL_MAGIC_1;
    header[2] = (unsigned char)BPL_MAGIC_2;
    header[3] = (unsigned char)BPL_MAGIC_3;
    header[4] = BPL_FORMAT_VER;
    header[5] = (unsigned char)lib->required_level;
    header[6] = (unsigned char)lib->ext_type;
    header[7] = 0; // flags
    bpl_write_le16(&header[8],
                   (unsigned int)lib->symbol_count);
    bpl_write_le16(&header[10],
                   (unsigned int)lib->src_line_count);
    // header[12-13] = string pool size (0 for source mode)
    bpl_write_le16(&header[12], 0);
    // Compute CRC over all source lines (accumulated)
    crc = 0xFFFF;
    for (int i = 0; i < lib->src_line_count; i++) {
        if (lib->src_lines[i].text) {
            crc = bpl_crc16_update(crc,
                (const unsigned char *)lib->src_lines[i].text,
                (int)strlen(lib->src_lines[i].text));
        }
    }
    bpl_write_le16(&header[14], crc);
    strncpy((char *)&header[16], lib->name, 16);

    if (fwrite(header, 1, BPL_HEADER_SIZE, fp) !=
        BPL_HEADER_SIZE) {
        printf("Write error on '%s'.\n", filename);
        fclose(fp);
        return -1;
    }

    // Write symbol table

    for (i = 0; i < lib->symbol_count; i++) {
        const LibSymbol *sym = &lib->symbols[i];
        unsigned char sym_rec[5 + MAX_LIB_NAME];
        int name_len = (int)strlen(sym->name);
        if (name_len > 63) name_len = 63;

        sym_rec[0] = (unsigned char)sym->type;
        sym_rec[1] = (unsigned char)sym->param_count;
        bpl_write_le16(&sym_rec[2],
                       (unsigned int)sym->entry_offset);
        sym_rec[4] = (unsigned char)name_len;
        memcpy(&sym_rec[5], sym->name, (size_t)name_len);

        if (fwrite(sym_rec, 1, (size_t)(5 + name_len), fp)
            != (size_t)(5 + name_len)) {
            printf("Write error at symbol %d.\n", i);
            fclose(fp);
            return -1;
        }
    }

    // Write source lines
    for (int i = 0; i < lib->src_line_count; i++) {
        const LibSourceLine *sl = &lib->src_lines[i];
        int text_len = sl->text ? (int)strlen(sl->text) : 0;
        unsigned char line_hdr[4];

        bpl_write_le16(&line_hdr[0],
                       (unsigned int)sl->vline);
        bpl_write_le16(&line_hdr[2],
                       (unsigned int)text_len);

        if (fwrite(line_hdr, 1, 4, fp) != 4) {
            printf("Write error at source line %d.\n", i);
            fclose(fp);
            return -1;
        }
        if (text_len > 0) {
            if ((int)fwrite(sl->text, 1,
                            (size_t)text_len, fp)
                != text_len) {
                printf("Write error at line %d text.\n", i);
                fclose(fp);
                return -1;
            }
        }
    }

    fclose(fp);
    printf("COMPILE: %s -> '%s' (%d symbols, %d lines)\n",
           lib->name, filename,
           lib->symbol_count, lib->src_line_count);
    return 0;
}

// --- bpl_load ---
int bpl_load(const char *filename, LoadedLibrary *lib)
{
    FILE *fp;
    unsigned char header[BPL_HEADER_SIZE];
    unsigned int sym_count, line_count;
    if (!lib) return -1;

    fp = fopen(filename, "rb");
    if (!fp) {
        printf("Cannot open '%s'.\n", filename);
        return -1;
    }

    // Read header
    if (fread(header, 1, BPL_HEADER_SIZE, fp) !=
        BPL_HEADER_SIZE) {
        printf("'%s' too short for .BPL.\n", filename);
        fclose(fp);
        return -1;
    }

    // Validate magic
    if (header[0] != (unsigned char)BPL_MAGIC_0 ||
        header[1] != (unsigned char)BPL_MAGIC_1 ||
        header[2] != (unsigned char)BPL_MAGIC_2 ||
        header[3] != (unsigned char)BPL_MAGIC_3) {
        printf("'%s' is not a .BPL file.\n", filename);
        fclose(fp);
        return -1;
    }

    // Validate version
    if (header[4] != BPL_FORMAT_VER) {
        printf("'%s' version %d unsupported (expected %d).\n",
               filename, (int)header[4], BPL_FORMAT_VER);
        fclose(fp);
        return -1;
    }

    // Extract header fields
    memset(lib, 0, sizeof(LoadedLibrary));
    lib->required_level = (SecLevel)header[5];
    lib->ext_type = (LibExtType)header[6];
    sym_count = bpl_read_le16(&header[8]);
    line_count = bpl_read_le16(&header[10]);
    strncpy(lib->name, (const char *)&header[16], 16);
    lib->name[16] = '\0';

    // Read symbol table
    for (int i = 0; i < (int)sym_count && i < MAX_LIB_SYMBOLS;
         i++) {
        unsigned char sym_hdr[5];
        int name_len;
        LibSymbol *sym = &lib->symbols[i];

        if (fread(sym_hdr, 1, 5, fp) != 5) {
            printf("Read error at symbol %d.\n", i);
            fclose(fp);
            return -1;
        }

        sym->type = (LibSymbolType)sym_hdr[0];
        sym->param_count = (int)sym_hdr[1];
        sym->entry_offset = (int)bpl_read_le16(&sym_hdr[2]);
        name_len = (int)sym_hdr[4];
        if (name_len >= MAX_LIB_NAME)
            name_len = MAX_LIB_NAME - 1;

        if (fread(sym->name, 1, (size_t)name_len, fp)
            != (size_t)name_len) {
            printf("Read error at symbol %d name.\n", i);
            fclose(fp);
            return -1;
        }
        sym->name[name_len] = '\0';
        sym->active = 1;
        lib->symbol_count++;
    }

    // Read source lines
    lib->src_line_cap = (int)line_count + 16;
    lib->src_lines = (LibSourceLine *)malloc(
        (size_t)lib->src_line_cap * sizeof(LibSourceLine));
    if (!lib->src_lines) {
        printf("Out of memory for source lines.\n");
        fclose(fp);
        return -1;
    }
    lib->src_line_count = 0;

    int i;
    for (i = 0; i < (int)line_count; i++) {
        unsigned char line_hdr[4];
        int vline, text_len;
        char *text;

        if (fread(line_hdr, 1, 4, fp) != 4) {
            printf("Read error at source line %d.\n", i);
            fclose(fp);
            return -1;
        }
        vline = (int)bpl_read_le16(&line_hdr[0]);
        text_len = (int)bpl_read_le16(&line_hdr[2]);

        text = (char *)malloc((size_t)(text_len + 1));
        if (!text) {
            printf("Out of memory at line %d.\n", i);
            fclose(fp);
            return -1;
        }
        if (text_len > 0) {
            if ((int)fread(text, 1, (size_t)text_len, fp)
                != text_len) {
                printf("Read error at line %d text.\n", i);
                free(text);
                fclose(fp);
                return -1;
            }
        }
        text[text_len] = '\0';

        strncpy(lib->src_lines[lib->src_line_count].text,
                text, 255);
        lib->src_lines[lib->src_line_count].text[255] = '\0';
        lib->src_lines[lib->src_line_count].vline = vline;
        lib->src_line_count++;
        free(text);
    }

    fclose(fp);

    // Validate CRC integrity
    {
        unsigned int stored_crc = bpl_read_le16(&header[14]);
        unsigned int computed_crc = 0xFFFF;
        for (int i = 0; i < lib->src_line_count; i++) {
            if (lib->src_lines[i].text[0] != '\0') {
                computed_crc = bpl_crc16_update(computed_crc,
                    (const unsigned char *)lib->src_lines[i].text,
                    (int)strlen(lib->src_lines[i].text));
            }
        }
        if (stored_crc != 0 && stored_crc != computed_crc) {
            printf("WARNING: CRC mismatch in '%s' "
                   "(stored=%04X, computed=%04X).\n",
                   filename, stored_crc, computed_crc);
        }
    }

    lib->loaded = 1;
    lib->compiled = 1;
    strncpy(lib->path, filename, 255);

    printf("BPLLOAD: %s (%d symbols, %d lines)\n",
           lib->name, lib->symbol_count, lib->src_line_count);
    return 0;
}
