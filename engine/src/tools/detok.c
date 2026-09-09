// FILENAME: detok.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (string.h)
// NEEDS: libengine (string.c, version.c)
// NEEDS: libkernel (version.h)
// Implements toolchain and compiler subsystem components for detok.
//
// ---- Includes ----

#include "platform/platform.h"
#include "hal/hal.h"
#include "types/version.h"
#include "runtime/format/snprintf.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include <stdbool.h>
#include <stdarg.h>

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

static int detok_getc(void *f) {
    unsigned char c;
    if (platform_file_read(f, &c, 1) == 1) return (int)c;
    return -1;
}

static void detok_puts(void *out, const char *str) {
    if (out) {
        platform_file_printf(out, "%s", str);
    } else {
        platform_console_puts(str);
    }
}

static void detok_putc(void *out, int c) {
    if (out) {
        char ch = (char)c;
        platform_file_write(out, &ch, 1);
    } else {
        platform_console_putchar(c);
    }
}

static void detok_printf(void *out, const char *fmt, ...) {
    char buf[512];
    va_list args;
    va_start(args, fmt);
    runtime_vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    if (out) {
        platform_file_printf(out, "%s", buf);
    } else {
        platform_console_puts(buf);
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        platform_console_printf("GW-BASIC Detokenizer (detok) - v%s\n", BASIC_VERSION_STRING);
        platform_console_puts("Usage: detok <input.bas> [output.txt]\n");
        return 1;
    }

    const char *infile = argv[1];
    void *in = platform_file_open(infile, "rb");
    if (!in) {
        platform_console_eprintf("Error: Could not open input file '%s'\n", infile);
        return 1;
    }

    void *out = NULL;
    if (argc >= 3) {
        out = platform_file_open(argv[2], "w");
        if (!out) {
            platform_file_close(in);
            platform_console_eprintf("Error: Could not open output file '%s'\n", argv[2]);
            return 1;
        }
    }

    // 1. Check Signature Byte
    int sig = detok_getc(in);
    if (sig != 0xFF) {
        platform_console_eputs("Warning: File does not start with signature byte 0xFF. Proceeding anyway.\n");
        if (sig != -1) {
            platform_file_seek(in, 0, IO_SEEK_SET);
        }
    }

    // 2. Decode lines
    while (true) {
        // Read next line offset pointer (2 bytes)
        int addr_low = detok_getc(in);
        int addr_high = detok_getc(in);
        if (addr_low == -1 || addr_high == -1) break;

        unsigned short next_addr = (unsigned short)(addr_low | (addr_high << 8));
        if (next_addr == 0x0000) {
            // End of program
            break;
        }

        // Read line number (2 bytes)
        int num_low = detok_getc(in);
        int num_high = detok_getc(in);
        if (num_low == -1 || num_high == -1) break;
        unsigned short line_num = (unsigned short)(num_low | (num_high << 8));

        detok_printf(out, "%u ", line_num);

        // Read statement characters/tokens until 0x00 (EOL)
        while (true) {
            int c = detok_getc(in);
            if (c == 0x00 || c == -1) {
                break;
            }

            if (c >= 0x80) {
                const char *kw = lookup_token((unsigned char)c);
                if (kw) {
                    detok_puts(out, kw);
                } else {
                    detok_printf(out, "[TOKEN:0x%02X]", c);
                }
            } else {
                detok_putc(out, c);
            }
        }
        detok_puts(out, "\n");
    }

    platform_file_close(in);
    if (out) {
        platform_file_close(out);
    } else {
        platform_console_flush();
    }

    return 0;
}
