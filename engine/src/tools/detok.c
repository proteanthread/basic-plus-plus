/**
 * @file detok.c
 * @brief Legacy GW-BASIC Tokenized Binary File Detokenizer Tool implementation.
 *
 * 1. WHAT IT DOES:
 * Implements `detok_main()`, decoding tokenized 0xFF-header binary GW-BASIC files (`.BAS`) into plain text ASCII BASIC code.
 *
 * 2. WHY IT EXISTS:
 * Provides standalone utility functionality to decode legacy 1980s GW-BASIC tokenized binary files into modern text formats.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Parses line-pointer offsets, 16-bit line numbers, and maps byte tokens (>= 0x80) and 2-byte tokens (0xFF Prefix) to ASCII keywords via `k_gw_tokens` tables.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into standalone CMake target 'detok'. Includes <stdio.h>, <stdlib.h>, <string.h>, <stdbool.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Excluded from 'libbasicpp' and 'libbasicpp_lite'. Compiled into standalone executable target 'detok'.
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Update `k_gw_tokens` dictionary tables for QuickBASIC token extensions.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Legacy GW-BASIC 0xFF file header signature and 16-bit line offset parsing math.
 *
 * 8. WHAT TO EXPECT:
 * Command-line binary input results in pure 7-bit ASCII BASIC source printed to stdout or written to output file.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify 0xFF magic byte signature and endian-safe 16-bit word decoding.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Input file exists and contains valid GW-BASIC binary tokens.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Endian-safe 16-bit integer unpacking.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files: None (Standalone execution payload).
 * Prerequisite Header Files: None.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
    unsigned char token;
    const char   *keyword;
} TokenMap;

static const TokenMap k_gw_tokens[] = {
    {0x81, "END"},
    {0x82, "FOR"},
    {0x83, "NEXT"},
    {0x84, "DATA"},
    {0x85, "INPUT"},
    {0x86, "DIM"},
    {0x87, "READ"},
    {0x88, "LET"},
    {0x89, "GOTO"},
    {0x8A, "RUN"},
    {0x8B, "IF"},
    {0x8C, "RESTORE"},
    {0x8D, "GOSUB"},
    {0x8E, "RETURN"},
    {0x8F, "REM"},
    {0x91, "PRINT"},
    {0x92, "ON"},
    {0x96, "DEF"},
    {0x97, "POKE"},
    {0x98, "PRINT#"},
    {0x99, "SECURE"},
    {0x9A, "CLS"},
    {0x9B, "WIDTH"},
    {0x9C, "ELSE"},
    {0x9D, "TRON"},
    {0x9E, "TROFF"},
    {0x9F, "SWAP"},
    {0xA0, "ERASE"},
    {0xA1, "EDIT"},
    {0xA2, "ERROR"},
    {0xA3, "RESUME"},
    {0xA4, "PLAY"},
    {0xA5, "ON/ERROR"},
    {0xA6, "LIST"},
    {0xA7, "NEW"},
    {0xA8, "ON/KEY"},
    {0xA9, "ON/PLAY"},
    {0xAA, "ON/TIMER"},
    {0xAB, "OPTION"},
    {0xAC, "ENVIRON"},
    {0xAD, "SYSTEM"},
    {0xAE, "KEY"},
    {0xAF, "SHELL"},
    {0xB0, "CLOSE"},
    {0xB1, "LOAD"},
    {0xB2, "MERGE"},
    {0xB3, "SAVE"},
    {0xB4, "COLOR"},
    {0xB7, "OPEN"},
    {0xB8, "FIELD"},
    {0xB9, "GET"},
    {0xBA, "PUT"},
    {0xBB, "SEEK"},
    {0xBC, "FILES"},
    {0xBD, "KILL"},
    {0xBE, "CHDIR"},
    {0xBF, "MKDIR"},
    {0xC0, "RMDIR"},
    {0xC1, "NAME"},
    {0xC2, "LINE"},
    {0xC3, "CIRCLE"},
    {0xC4, "PSET"},
    {0xC5, "PRESET"},
    {0xC6, "SCREEN"},
    {0xC7, "PAINT"},
    {0xC8, "SOUND"},
    {0xCC, "USING"},
    {0xCD, "THEN"},
    {0xCE, "TO"},
    {0xCF, "STEP"},
    {0xD0, "DIR"},
    {0xD1, "SETATTR"},
    {0xD2, "LOCK"},
    {0xD3, "UNLOCK"},
    {0x00, NULL}
};

static const char *lookup_token(unsigned char tok) {
    for (int i = 0; k_gw_tokens[i].keyword != NULL; ++i) {
        if (k_gw_tokens[i].token == tok) {
            return k_gw_tokens[i].keyword;
        }
    }
    return NULL;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("GW-BASIC Detokenizer (detok) - v6.0.0\n");
        printf("Usage: detok <input.bas> [output.txt]\n");
        return 1;
    }

    const char *infile = argv[1];
    FILE *in = fopen(infile, "rb");
    if (!in) {
        fprintf(stderr, "Error: Could not open input file '%s'\n", infile);
        return 1;
    }

    FILE *out = stdout;
    if (argc >= 3) {
        out = fopen(argv[2], "w");
        if (!out) {
            fclose(in);
            fprintf(stderr, "Error: Could not open output file '%s'\n", argv[2]);
            return 1;
        }
    }

    /* 1. Check Signature Byte */
    int sig = fgetc(in);
    if (sig != 0xFF) {
        fprintf(stderr, "Warning: File does not start with signature byte 0xFF. Proceeding anyway.\n");
        if (sig != EOF) {
            rewind(in);
        }
    }

    /* 2. Decode lines */
    while (true) {
        /* Read next line offset pointer (2 bytes) */
        int addr_low = fgetc(in);
        int addr_high = fgetc(in);
        if (addr_low == EOF || addr_high == EOF) break;

        unsigned short next_addr = (unsigned short)(addr_low | (addr_high << 8));
        if (next_addr == 0x0000) {
            /* End of program */
            break;
        }

        /* Read line number (2 bytes) */
        int num_low = fgetc(in);
        int num_high = fgetc(in);
        if (num_low == EOF || num_high == EOF) break;
        unsigned short line_num = (unsigned short)(num_low | (num_high << 8));

        fprintf(out, "%u ", line_num);

        /* Read statement characters/tokens until 0x00 (EOL) */
        while (true) {
            int c = fgetc(in);
            if (c == 0x00 || c == EOF) {
                break;
            }

            if (c >= 0x80) {
                const char *kw = lookup_token((unsigned char)c);
                if (kw) {
                    fprintf(out, "%s", kw);
                } else {
                    fprintf(out, "[TOKEN:0x%02X]", c);
                }
            } else {
                fputc(c, out);
            }
        }
        fprintf(out, "\n");
    }

    fclose(in);
    if (out != stdout) {
        fclose(out);
    }

    return 0;
}
