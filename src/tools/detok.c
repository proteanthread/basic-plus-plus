/**
 * @file detok.c
 * @brief GW-BASIC binary file detokenizer tool.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Decodes tokenized GW-BASIC binary program files (.bas) back into human-readable ASCII text.
 * - Why it exists: Fulfills the requirement to decode legacy binary-encoded GW-BASIC programs.
 * - Why it works this way: Legacy files start with 0xFF signature byte. For each line, it reads the next line pointer,
 *   line number (2-byte unsigned short), and translates token code bytes (>= 0x80) back to keywords using a mapping table.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Token keyword dictionary mappings.
 * - What cannot be changed: Structural pointer offsets decoding.
 * - What to expect: Safely detokenizing standard GW-BASIC files.
 * - What to do if something breaks: If keywords detokenize to gibberish, verify token hex mappings.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Input file is binary formatted GW-BASIC.
 * - Portability concerns: Multi-byte reads must be handled in an endian-safe manner.
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
