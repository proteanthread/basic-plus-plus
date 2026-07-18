/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: trans.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Standalone transpiler command line tool for BASIC++ (trans).
 *    Transpiles BASIC source files to C17, C89, Python 3, Free Pascal, or Fortran.
 * ===================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../config.h"
#include "../platform.h"
#include "../ast.h"
#include "../lexer.h"
#include "../memory.h"
#include "../errors.h"
#include "../codegen/trans.h"

#include "../boot.h"
#include "../runtime.h"
#include "../codegen/target.h"

static bool g_debug_mode = false;

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
    {0x00, NULL}
};

static const char *lookup_token(unsigned char tok) {
    int i;
    for (i = 0; k_gw_tokens[i].keyword != NULL; ++i) {
        if (k_gw_tokens[i].token == tok) {
            return k_gw_tokens[i].keyword;
        }
    }
    return NULL;
}

static char *detokenize_gw_basic(const unsigned char *data, size_t file_len) {
    size_t out_cap = file_len * 5 + 1024;
    char *out = malloc(out_cap);
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
        
        if (next_addr == 0x0000) {
            break;
        }
        
        if (p + 2 > file_len) break;
        uint16_t line_num = (uint16_t)(data[p] | (data[p + 1] << 8));
        p += 2;
        
        int written = snprintf(out + out_len, out_cap - out_len, "%u ", line_num);
        if (written > 0) out_len += written;
        
        while (p < file_len) {
            unsigned char c = data[p++];
            if (c == 0x00) {
                break;
            }
            
            if (c >= 0x80) {
                const char *kw = lookup_token(c);
                if (kw) {
                    size_t kw_len = strlen(kw);
                    if (out_len + kw_len < out_cap) {
                        strcpy(out + out_len, kw);
                        out_len += kw_len;
                    }
                } else {
                    int w = snprintf(out + out_len, out_cap - out_len, "[TOKEN:0x%02X]", c);
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

// Simple file loader that reads lines and stores them in ProgramStore
static int load_program_file(const char *filename, ProgramStore *store) {
    FILE *f = fopen(filename, "rb");
    if (!f) return -1;
    
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    unsigned char *fdata = malloc(fsize >= 0 ? fsize + 1 : 1);
    if (!fdata) {
        fclose(f);
        return -2;
    }
    
    if (fsize > 0) {
        if (fread(fdata, 1, fsize, f) != (size_t)fsize) {
            fclose(f);
            free(fdata);
            return -3;
        }
    }
    fclose(f);
    
    char *source_text = NULL;
    if (fsize > 0 && fdata[0] == 0xFF) {
        if (g_debug_mode) {
            printf("[DEBUG] Detected tokenized GW-BASIC binary format. Decoding...\n");
        }
        source_text = detokenize_gw_basic(fdata, fsize);
        free(fdata);
        if (!source_text) return -4;
    } else {
        fdata[fsize] = '\0';
        source_text = (char *)fdata;
    }
    
    store->count = 0;
    store->capacity = 1024;
    store->lines = malloc(sizeof(ProgramLine) * store->capacity);
    if (!store->lines) {
        free(source_text);
        return -2;
    }
    
    bool has_numbers = false;
    const char *scan = source_text;
    while (*scan) {
        while (*scan && (*scan == ' ' || *scan == '\t' || *scan == '\r' || *scan == '\n')) {
            scan++;
        }
        if (*scan >= '0' && *scan <= '9') {
            has_numbers = true;
            break;
        }
        while (*scan && *scan != '\n') {
            scan++;
        }
        if (*scan == '\n') scan++;
    }

    const char *p = source_text;
    while (*p) {
        char line_buf[2048];
        size_t idx = 0;
        while (*p && *p != '\n' && idx < sizeof(line_buf) - 1) {
            line_buf[idx++] = *p++;
        }
        line_buf[idx] = '\0';
        if (*p == '\n') p++;
        
        // Strip trailing newline/cr
        size_t len = strlen(line_buf);
        while (len > 0 && (line_buf[len - 1] == '\r' || line_buf[len - 1] == '\n')) {
            line_buf[len - 1] = '\0';
            len--;
        }
        if (len == 0) continue;
        
        char final_line[2048];
        double line_num;
        if (has_numbers) {
            const char *pl = line_buf;
            while (*pl && (*pl == ' ' || *pl == '\t')) pl++;
            if (!*pl) continue;
            line_num = atof(pl);
            if (line_num <= 0) {
                continue;
            }
            strcpy(final_line, line_buf);
        } else {
            line_num = (store->count + 1) * 10;
            snprintf(final_line, sizeof(final_line), "%.0f %s", line_num, line_buf);
        }
        
        if (store->count >= store->capacity) {
            store->capacity *= 2;
            ProgramLine *new_lines = realloc(store->lines, sizeof(ProgramLine) * store->capacity);
            if (!new_lines) {
                free(source_text);
                return -2;
            }
            store->lines = new_lines;
        }
        
        store->lines[store->count].line_number = line_num;
        store->lines[store->count].text = plat_strdup(final_line);
        store->count++;
    }
    
    free(source_text);
    return 0;
}

// Global variable required by the parser is already defined in errors.c

int main(int argc, char **argv)
{
    TransTarget target_lang = TRANS_TARGET_C17;
    const char *infile = NULL;
    const char *outfile = "output.out";
    const char *platform_target = NULL;
    int i;

    if (argc < 2) {
        printf("BASIC++ Transpiler (trans) v" BASICPP_VERSION "\n\n");
        printf("Usage:\n");
        printf("  trans [switches] <input.bas> [-o <output_file>]\n\n");
        printf("Switches:\n");
        printf("  --C89            Translate directly to standard, portable, pure C89.\n");
        printf("  --C17            Translate directly to standard, portable, pure C17 (default).\n");
        printf("  --freedos        Emit 16-bit DOS platform headers and data constraints.\n");
        printf("  --windows        Emit standard 64-bit Windows platform headers and definitions.\n");
        printf("  --linux          Emit standard 64-bit Linux platform headers and macros.\n");
        printf("  --help, -h, -?   Display this command usage information.\n");
        printf("  --about          Display information about the transpiler features.\n");
        printf("  --license        Display the software license details.\n");
        printf("  --version        Display transpiler version details.\n");
        printf("  --debug          Output verbose diagnostics during the transpilation process.\n");
        return 0;
    }

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-?") == 0) {
            printf("BASIC++ Transpiler (trans) v" BASICPP_VERSION "\n\n");
            printf("Usage:\n");
            printf("  trans [switches] <input.bas> [-o <output_file>]\n\n");
            printf("Switches:\n");
            printf("  --C89            Translate directly to standard, portable, pure C89.\n");
            printf("  --C17            Translate directly to standard, portable, pure C17 (default).\n");
            printf("  --freedos        Emit 16-bit DOS platform headers and data constraints.\n");
            printf("  --windows        Emit standard 64-bit Windows platform headers and definitions.\n");
            printf("  --linux          Emit standard 64-bit Linux platform headers and macros.\n");
            printf("  --help, -h, -?   Display this command usage information.\n");
            printf("  --about          Display information about the transpiler features.\n");
            printf("  --license        Display the software license details.\n");
            printf("  --version        Display transpiler version details.\n");
            printf("  --debug          Output verbose diagnostics during the transpilation process.\n");
            return 0;
        } else if (strcmp(argv[i], "--about") == 0) {
            printf("BASIC++ Transpiler (trans)\n");
            printf("Transpiles BASIC source scripts (.bas) and intermediate bytecode payloads\n");
            printf("into portable high-performance source files in standard C17, C89, Python 3, Free Pascal, or Fortran.\n");
            return 0;
        } else if (strcmp(argv[i], "--license") == 0) {
            printf("Modified MIT License\n\n");
            printf("Copyright (c) 2025 BASIC++ Community\n\n");
            printf("Permission is hereby granted, free of charge, to any person obtaining a copy\n");
            printf("of this software and associated documentation files (the \"Software\"), to deal\n");
            printf("in the Software without restriction, including without limitation the rights\n");
            printf("to use, copy, modify, and/or sell copies of the Software, and to permit persons\n");
            printf("to whom the Software is furnished to do so, subject to the following conditions:\n\n");
            printf("The above copyright notice and this permission notice shall be included in all\n");
            printf("copies or substantial portions of the Software.\n\n");
            printf("THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n");
            printf("IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n");
            printf("FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\n");
            printf("AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n");
            printf("LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\n");
            printf("OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE\n");
            printf("SOFTWARE.\n\n");
            printf("THIS SOFTWARE IS NOT TO BE SOLD.\n");
            return 0;
        } else if (strcmp(argv[i], "--version") == 0) {
            printf("trans version %s (synchronized with BASIC++ %s)\n", BASICPP_VERSION, BASICPP_VERSION);
            return 0;
        } else if (strcmp(argv[i], "--debug") == 0) {
            g_debug_mode = true;
        } else if (strcmp(argv[i], "--C89") == 0) {
            target_lang = TRANS_TARGET_C89;
        } else if (strcmp(argv[i], "--C17") == 0) {
            target_lang = TRANS_TARGET_C17;
        } else if (strcmp(argv[i], "--freedos") == 0) {
            platform_target = "freedos";
        } else if (strcmp(argv[i], "--windows") == 0) {
            platform_target = "windows";
        } else if (strcmp(argv[i], "--linux") == 0) {
            platform_target = "linux";
        } else if (strcmp(argv[i], "--target") == 0 && i + 1 < argc) {
            const char *t = argv[++i];
            if (strcmp(t, "c17") == 0) target_lang = TRANS_TARGET_C17;
            else if (strcmp(t, "c89") == 0) target_lang = TRANS_TARGET_C89;
            else if (strcmp(t, "py3") == 0) target_lang = TRANS_TARGET_PY3;
            else if (strcmp(t, "pas") == 0) target_lang = TRANS_TARGET_PAS;
            else if (strcmp(t, "f90") == 0) target_lang = TRANS_TARGET_F90;
            else {
                fprintf(stderr, "Unknown target language: %s\n", t);
                return 1;
            }
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            outfile = argv[++i];
        } else if (argv[i][0] != '-') {
            infile = argv[i];
        }
    }

    if (!infile) {
        fprintf(stderr, "Error: Input file parameter is required.\n");
        return 1;
    }

    if (g_debug_mode) {
        printf("[DEBUG] Target Language: %d\n", target_lang);
        printf("[DEBUG] Platform Target Config: %s\n", platform_target ? platform_target : "default");
        printf("[DEBUG] Loading program file: %s\n", infile);
    }

    BootConfig boot_cfg;
    memset(&boot_cfg, 0, sizeof(boot_cfg));
    boot_cfg.verbosity = BOOT_SILENT;

    MemorySystem memory;
    static RuntimeState runtime;
    if (boot_execute(&boot_cfg, &memory, &runtime) == BOOT_CRITICAL) {
        fprintf(stderr, "Failed to initialize bootstrap engine.\n");
        return 1;
    }

    keyword_registry_init();

    ProgramStore program = {0};
    if (load_program_file(infile, &program) != 0) {
        fprintf(stderr, "Failed to load input file '%s'\n", infile);
        return 1;
    }
    strncpy(program.filename, infile, sizeof(program.filename) - 1);
    program.filename[sizeof(program.filename) - 1] = '\0';

    if (program.count == 0) {
        fprintf(stderr, "Input file '%s' contains no numbered lines.\n", infile);
        return 1;
    }

    AstLine *ast_lines = malloc(sizeof(AstLine) * program.count);
    if (!ast_lines) {
        fprintf(stderr, "Out of memory allocating AST lines.\n");
        return 1;
    }
    memset(ast_lines, 0, sizeof(AstLine) * program.count);

    if (g_debug_mode) {
        printf("[DEBUG] Parsing AST line by line (%d lines)...\n", program.count);
    }

    int success = 1;
    for (i = 0; i < program.count; i++) {
        Lexer lex;
        ProgramLine *pl = &program.lines[i];

        ast_lines[i].line_number = pl->line_number;

        if (g_debug_mode) {
            printf("[DEBUG] Parsing line %.0f: \"%s\"\n", pl->line_number, pl->text);
        }

        lexer_init(&lex, pl->text);

        // Skip the line number token
        if (lex.current.type == TOK_NUMBER || lex.current.type == TOK_FLOAT_LIT) {
            lexer_next(&lex);
        }

        error_clear();
        g_current_executing_line = pl->line_number;

        ast_lines[i].stmts = ast_build_line(&lex, (int)pl->line_number);

        if (error_occurred()) {
            printf("Compile error at line %.0f\n", pl->line_number);
            success = 0;
            error_clear();
            break;
        }
    }

    int ret = -1;
    if (success) {
        if (g_debug_mode) {
            printf("[DEBUG] Opening output file: %s\n", outfile);
        }
        FILE *out = fopen(outfile, "w");
        if (!out) {
            perror("Failed to open output file");
            return 1;
        }

        const TargetConfig *config = NULL;
        if (platform_target) {
            config = target_find(platform_target);
        }
        if (!config) {
#ifdef _WIN32
            config = target_find("windows");
#else
            config = target_find("linux");
#endif
        }
        if (!config) {
            config = target_get_default();
        }

        if (g_debug_mode) {
            printf("[DEBUG] Emitting code to target...\n");
        }
        ret = trans_emit(out, ast_lines, program.count, &program, config, target_lang);
        fclose(out);
    }

    // Cleanup
    g_current_executing_line = 0.0;
    for (i = 0; i < program.count; i++) {
        if (ast_lines[i].stmts) {
            ast_free_line(ast_lines[i].stmts);
        }
        free(program.lines[i].text);
    }
    free(ast_lines);
    free(program.lines);

    if (ret == 0) {
        printf("Transpilation completed successfully.\n");
    } else {
        printf("Transpilation failed.\n");
    }

    return ret;
}
