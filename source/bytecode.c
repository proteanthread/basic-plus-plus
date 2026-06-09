/*
 * =====================================================================
 * BASIC++ Interpreter - bytecode.c
 * =====================================================================
 *
 * Bytecode format (.BPP) serializer/deserializer - Phase 13.
 *
 * IMPLEMENTATION:
 *   The .bpp format stores program lines in a binary container
 *   with a 16-byte version-tagged header. Each line is stored as:
 *     [2-byte line number LE] [2-byte text length LE] [text bytes]
 *
 *   The source text is stored verbatim (not further tokenized),
 *   because the interpreter's Lexer re-tokenizes on execution.
 *   This keeps the format simple and maximally compatible.
 *
 *   Binary I/O uses fread/fwrite with unsigned char buffers and
 *   manual byte packing to avoid struct alignment issues across
 *   compilers. All multi-byte values are little-endian.
 *
 * ANSI C89/C90 COMPLIANT
 * =====================================================================
 */

#include <stdio.h>
#include <string.h>
#include "bytecode.h"
#include "dialect.h"
#include "config.h"

/* =====================================================================
 * Little-endian helpers
 * =====================================================================
 * Manual byte packing avoids endianness and alignment issues.
 */
static void write_le16(unsigned char *buf, unsigned int val)
{
    buf[0] = (unsigned char)(val & 0xFF);
    buf[1] = (unsigned char)((val >> 8) & 0xFF);
}

static unsigned int read_le16(const unsigned char *buf)
{
    return (unsigned int)buf[0] |
           ((unsigned int)buf[1] << 8);
}

/* =====================================================================
 * bpp_save - Serialize program store to .bpp file.
 * =====================================================================
 */
int bpp_save(const ProgramStore *prog, const char *filename)
{
    FILE *fp;
    unsigned char header[BPP_HEADER_SIZE];
    int i;

    if (!prog || prog->count == 0) {
        printf("No program to save.\n");
        return -1;
    }

    if (!filename || filename[0] == '\0') {
        printf("No filename specified.\n");
        return -1;
    }

    fp = fopen(filename, "wb");
    if (!fp) {
        printf("Cannot open '%s' for writing.\n", filename);
        return -1;
    }

    /* Build header */
    memset(header, 0, BPP_HEADER_SIZE);
    header[0] = (unsigned char)BPP_MAGIC_0;
    header[1] = (unsigned char)BPP_MAGIC_1;
    header[2] = (unsigned char)BPP_MAGIC_2;
    header[3] = (unsigned char)BPP_MAGIC_3;
    header[4] = BPP_FORMAT_VER;
    header[5] = (unsigned char)dialect_get_config()->id;
    /* header[6..7] = flags (0) */
    write_le16(&header[8], (unsigned int)prog->count);
    /* header[10..15] = reserved (0) */

    /* Write header */
    if (fwrite(header, 1, BPP_HEADER_SIZE, fp) != BPP_HEADER_SIZE) {
        printf("Write error on '%s'.\n", filename);
        fclose(fp);
        return -1;
    }

    /* Write each line record */
    for (i = 0; i < prog->count; i++) {
        const ProgramLine *pl = &prog->lines[i];
        unsigned char rec[4];
        int text_len;

        text_len = (int)strlen(pl->text);

        /* Line number (LE16) */
        write_le16(&rec[0], (unsigned int)pl->line_number);
        /* Text length (LE16) */
        write_le16(&rec[2], (unsigned int)text_len);

        if (fwrite(rec, 1, 4, fp) != 4) {
            printf("Write error at line %d.\n", pl->line_number);
            fclose(fp);
            return -1;
        }

        /* Text bytes (no NUL terminator) */
        if (text_len > 0) {
            if ((int)fwrite(pl->text, 1, (size_t)text_len, fp)
                    != text_len) {
                printf("Write error at line %d.\n",
                       pl->line_number);
                fclose(fp);
                return -1;
            }
        }
    }

    fclose(fp);
    printf("BSAVE: %d lines to '%s' (%s)\n",
           prog->count, filename,
           dialect_get_config()->short_name);
    return 0;
}

/* =====================================================================
 * bpp_load - Deserialize .bpp file into program store.
 * =====================================================================
 */
int bpp_load(ProgramStore *prog, const char *filename)
{
    FILE *fp;
    unsigned char header[BPP_HEADER_SIZE];
    unsigned int line_count;
    int i;

    if (!filename || filename[0] == '\0') {
        printf("No filename specified.\n");
        return -1;
    }

    fp = fopen(filename, "rb");
    if (!fp) {
        printf("Cannot open '%s' for reading.\n", filename);
        return -1;
    }

    /* Read header */
    if (fread(header, 1, BPP_HEADER_SIZE, fp) != BPP_HEADER_SIZE) {
        printf("'%s' is too short - not a .bpp file.\n", filename);
        fclose(fp);
        return -1;
    }

    /* Validate magic */
    if (header[0] != (unsigned char)BPP_MAGIC_0 ||
        header[1] != (unsigned char)BPP_MAGIC_1 ||
        header[2] != (unsigned char)BPP_MAGIC_2 ||
        header[3] != (unsigned char)BPP_MAGIC_3) {
        printf("'%s' is not a .bpp file (bad magic).\n", filename);
        fclose(fp);
        return -1;
    }

    /* Validate version */
    if (header[4] != BPP_FORMAT_VER) {
        printf("'%s' has unsupported version %d "
               "(expected %d).\n",
               filename, (int)header[4], BPP_FORMAT_VER);
        fclose(fp);
        return -1;
    }

    line_count = read_le16(&header[8]);

    if ((int)line_count > prog->capacity) {
        printf("'%s' has %u lines - exceeds capacity (%d).\n",
               filename, line_count, prog->capacity);
        fclose(fp);
        return -1;
    }

    /* Clear existing program */
    program_clear(prog);

    /* Read each line record */
    for (i = 0; i < (int)line_count; i++) {
        unsigned char rec[4];
        unsigned int linenum, textlen;
        char text_buf[MAX_LINE_LENGTH + 1];

        if (fread(rec, 1, 4, fp) != 4) {
            printf("Read error at record %d in '%s'.\n",
                   i, filename);
            fclose(fp);
            return -1;
        }

        linenum = read_le16(&rec[0]);
        textlen = read_le16(&rec[2]);

        if (textlen > MAX_LINE_LENGTH) {
            printf("Line %u in '%s' too long (%u bytes).\n",
                   linenum, filename, textlen);
            fclose(fp);
            return -1;
        }

        if (textlen > 0) {
            if (fread(text_buf, 1, (size_t)textlen, fp)
                    != (size_t)textlen) {
                printf("Read error at line %u in '%s'.\n",
                       linenum, filename);
                fclose(fp);
                return -1;
            }
        }
        text_buf[textlen] = '\0';

        /* Insert into program store */
        if (program_insert(prog, (int)linenum, text_buf) != 0) {
            printf("Cannot insert line %u - store full.\n",
                   linenum);
            fclose(fp);
            return -1;
        }
    }

    fclose(fp);
    printf("BLOAD: %u lines from '%s'\n", line_count, filename);

    /* Optionally switch dialect to match the file */
    {
        int file_dialect = (int)header[5];
        int curr_dialect = (int)dialect_get_config()->id;
        if (file_dialect != curr_dialect &&
            file_dialect >= 0 && file_dialect < DIALECT_COUNT) {
            dialect_init((DialectId)file_dialect);
            dialect_apply();
            printf("Dialect: %s [%s]\n",
                   dialect_get_config()->name,
                   dialect_get_config()->short_name);
        }
    }

    return 0;
}
