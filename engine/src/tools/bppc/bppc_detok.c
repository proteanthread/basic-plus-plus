// FILENAME: bppc_detok.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (bppc_internal.h)
// Implements toolchain and compiler subsystem components for bppc_detok.
//
// ---- Includes ----

#include "tools/bppc_internal.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "runtime/memory/alloc.h"

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

char *detokenize_gw_basic(const unsigned char *data, size_t file_len) {
    size_t out_cap = file_len * 5 + 1024;
    char *out = (char *)runtime_calloc(1, out_cap);
    if (!out) return NULL;
    
    size_t out_len = 0;
    size_t p = 0;
    if (file_len > 0 && data[0] == 0xFF) {
        p++;
    }
    
    while (p < file_len) {
        if (p + 2 > file_len) break;
        uint16_t next_addr = (uint16_t)(data[p] | (data[p + 1] << 8));
        p += 2;
        
        if (next_addr == 0x0000) break;
        if (p + 2 > file_len) break;

        uint16_t line_num = (uint16_t)(data[p] | (data[p + 1] << 8));
        p += 2;
        
        int written = runtime_snprintf(out + out_len, out_cap - out_len, "%u ", line_num);
        if (written > 0) out_len += written;
        
        while (p < file_len) {
            unsigned char c = data[p++];
            if (c == 0x00) break;
            
            if (c >= 0x80) {
                const char *kw = lookup_token(c);
                if (kw) {
                    size_t kw_len = runtime_strlen(kw);
                    if (out_len + kw_len < out_cap) {
                        runtime_memcpy(out + out_len, kw, kw_len);
                        out_len += kw_len;
                    }
                } else {
                    int w = runtime_snprintf(out + out_len, out_cap - out_len, "[TOKEN:0x%02X]", c);
                    if (w > 0) out_len += w;
                }
            } else {
                if (out_len + 1 < out_cap) {
                    out[out_len++] = (char)c;
                }
            }
        }
        if (out_len + 1 < out_cap) {
            out[out_len++] = '\n';
        }
    }
    out[out_len] = '\0';
    return out;
}
