/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ — a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file bppc.c
 * @brief BASIC++ to C17 Transpiler and Compiler tool implementation.
 *
 * 1. WHAT IT DOES:
 * Implements `bppc_main()`, transpiling BASIC++ source code (`.bas`) into portable, native C17 source code (`.c`) or appending bytecode to a C17 VM stub.
 *
 * 2. WHY IT EXISTS:
 * Serves as the official compiler target (`bppc`/`bppc.exe`) producing high-performance native executables from BASIC++ scripts.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Translates line numbers and control flow (`GOTO`, `GOSUB`, `FOR`, `WHILE`) into native C labels, switch loops, and runtime library function calls (`libbasicpp`).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into standalone CMake target 'bppc'. Includes "types/version.h", "types/config.h", <stdio.h>, <stdlib.h>, <string.h>, <ctype.h>, <stdbool.h>, <stdint.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Excluded from 'libbasicpp' and 'libbasicpp_lite'. Compiled into standalone executable target 'bppc'.
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Extend codegen output in `transpile_line()` to translate new BASIC++ statements into C17 calls.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * C17 codegen compliance invariant — transpiled `.c` output must compile without warnings on GCC, Clang, and MSVC.
 *
 * 8. WHAT TO EXPECT:
 * Input BASIC source produces clean C17 code featuring explicit line-number labels and static allocations.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Inspect generated `.c` code for missing label declarations or unmatched loop scope braces.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Source file parsed contains valid BASIC++ syntax.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Transpiled C source links against `libbasicpp`.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/types/config.c
 * Prerequisite Header Files:
 * - engine/include/types/version.h
 * - engine/include/types/config.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include "types/version.h"

#include "types/config.h"

static bool g_debug_mode = false;

static char *trim(char *str) {
    char *end;
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

static bool parse_line_number(const char *str, int *out_line, const char **out_text) {
    while (*str && isspace((unsigned char)*str)) str++;
    if (!isdigit((unsigned char)*str)) return false;
    char *endptr;
    long val = strtol(str, &endptr, 10);
    *out_line = (int)val;
    *out_text = endptr;
    return true;
}



/* Legcy C Transpiler removed. Use dedicated trans tool. */



static char *read_file_to_string(const char *path) {
    FILE *fp = fopen(path, "rb");
    if (!fp) return NULL;
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    if (size < 0) {
        fclose(fp);
        return NULL;
    }
    fseek(fp, 0, SEEK_SET);
    char *buf = calloc(size + 1, 1);
    if (!buf) {
        fclose(fp);
        return NULL;
    }
    size_t read_bytes = fread(buf, 1, size, fp);
    buf[read_bytes] = '\0';
    fclose(fp);
    return buf;
}



static void write_le16(unsigned char *buf, uint16_t val) {
    buf[0] = (unsigned char)(val & 0xFF);
    buf[1] = (unsigned char)((val >> 8) & 0xFF);
}

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

static char *detokenize_gw_basic(const unsigned char *data, size_t file_len) {
    size_t out_cap = file_len * 5 + 1024;
    char *out = (char *)calloc(1, out_cap);
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
                        memcpy(out + out_len, kw, kw_len);
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

typedef struct {
    uint16_t line_num;
    char *text;
} CompiledLine;

static char *bppc_strdup(const char *src) {
    size_t len = strlen(src);
    char *dst = (char *)calloc(1, len + 1);
    if (dst) {
        memcpy(dst, src, len + 1);
    }
    return dst;
}

static int compare_compiled_lines(const void *a, const void *b) {
    uint16_t la = ((CompiledLine *)a)->line_num;
    uint16_t lb = ((CompiledLine *)b)->line_num;
    return (la < lb) ? -1 : (la > lb) ? 1 : 0;
}

static bool compile_to_bpp(const char *source_text, const char *out_bpp_path) {
    if (g_debug_mode) {
        printf("[DEBUG] Starting bytecode compilation to: '%s'\n", out_bpp_path);
    }
    size_t line_cap = 256;
    size_t line_count = 0;
    CompiledLine *lines = (CompiledLine *)calloc(line_cap, sizeof(CompiledLine));
    if (!lines) return false;
    
    const char *p = source_text;
    uint16_t auto_line = 10;
    
    while (*p) {
        char line_buf[2048];
        size_t idx = 0;
        while (*p && *p != '\n' && idx < sizeof(line_buf) - 1) {
            line_buf[idx++] = *p++;
        }
        line_buf[idx] = '\0';
        if (*p == '\n') p++;
        
        char *trimmed = trim(line_buf);
        if (trimmed[0] == '\0') continue;
        
        uint16_t line_num = 0;
        const char *stmt_text = NULL;
        int lnum_parsed = 0;
        if (parse_line_number(trimmed, &lnum_parsed, &stmt_text)) {
            line_num = (uint16_t)lnum_parsed;
            char temp[2048];
            strncpy(temp, stmt_text, sizeof(temp) - 1);
            temp[sizeof(temp) - 1] = '\0';
            trimmed = trim(temp);
        } else {
            line_num = auto_line;
            auto_line += 10;
        }
        
        if (g_debug_mode) {
            printf("[DEBUG] Parsed line %d: \"%s\"\n", (int)line_num, trimmed);
        }
        
        if (line_count >= line_cap) {
            line_cap *= 2;
            CompiledLine *new_lines = realloc(lines, line_cap * sizeof(CompiledLine));
            if (!new_lines) {
                for (size_t i = 0; i < line_count; ++i) free(lines[i].text);
                free(lines);
                return false;
            }
            lines = new_lines;
        }
        
        lines[line_count].line_num = line_num;
        lines[line_count].text = bppc_strdup(trimmed);
        line_count++;
    }
    
    if (g_debug_mode) {
        printf("[DEBUG] Sorting %d compiled lines by line number...\n", (int)line_count);
    }
    qsort(lines, line_count, sizeof(CompiledLine), compare_compiled_lines);
    
    if (g_debug_mode) {
        printf("[DEBUG] Writing bytecode output to '%s'...\n", out_bpp_path);
    }
    FILE *fout = fopen(out_bpp_path, "wb");
    if (!fout) {
        for (size_t i = 0; i < line_count; ++i) free(lines[i].text);
        free(lines);
        return false;
    }
    
    unsigned char header[16];
    memset(header, 0, sizeof(header));
    header[0] = 'B';
    header[1] = 'P';
    header[2] = 'P';
    header[3] = 0x1A;
    header[4] = 1;
    header[5] = 2;
    write_le16(&header[8], (uint16_t)line_count);
    
    if (g_debug_mode) {
        printf("[DEBUG] Writing 16-byte BPP file header...\n");
    }
    fwrite(header, 1, 16, fout);
    
    for (size_t i = 0; i < line_count; ++i) {
        uint16_t text_len = (uint16_t)strlen(lines[i].text);
        unsigned char rec[4];
        write_le16(&rec[0], lines[i].line_num);
        write_le16(&rec[2], text_len);
        if (g_debug_mode) {
            printf("[DEBUG] Writing record: line=%d, len=%d, text=\"%s\"\n", (int)lines[i].line_num, (int)text_len, lines[i].text);
        }
        fwrite(rec, 1, 4, fout);
        if (text_len > 0) {
            fwrite(lines[i].text, 1, text_len, fout);
        }
    }
    
    fclose(fout);
    for (size_t i = 0; i < line_count; ++i) free(lines[i].text);
    free(lines);
    if (g_debug_mode) {
        printf("[DEBUG] Bytecode compilation completed successfully.\n");
    }
    return true;
}

static bool package_standalone(const char *basic_path, const char *out_exe_path, const char *argv0, const char *stub_filename) {
    if (g_debug_mode) {
        printf("[DEBUG] Packaging standalone binary: '%s' using stub: '%s'\n", out_exe_path, stub_filename);
    }
    char stub_path[1024] = "";
    const char *last_slash = strrchr(argv0, '/');
    if (!last_slash) last_slash = strrchr(argv0, '\\');
    
    if (last_slash) {
        size_t dir_len = last_slash - argv0 + 1;
        memcpy(stub_path, argv0, dir_len);
        stub_path[dir_len] = '\0';
        snprintf(stub_path + dir_len, sizeof(stub_path) - dir_len, "%s", stub_filename);
    } else {
        snprintf(stub_path, sizeof(stub_path), "%s", stub_filename);
    }
    
    if (g_debug_mode) {
        printf("[DEBUG] Attempting to open stub at path: '%s'\n", stub_path);
    }
    FILE *fstub = fopen(stub_path, "rb");
    if (!fstub) {
        fstub = fopen(stub_filename, "rb");
        if (!fstub) {
            fprintf(stderr, "Error: Could not locate '%s' runner stub (expected in '%s' or current directory)\n", stub_filename, stub_path);
            return false;
        }
    }
    
    FILE *fout = fopen(out_exe_path, "wb");
    if (!fout) {
        fclose(fstub);
        fprintf(stderr, "Error: Could not open output executable '%s' for writing\n", out_exe_path);
        return false;
    }
    
    if (g_debug_mode) {
        printf("[DEBUG] Copying stub to output binary...\n");
    }
    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), fstub)) > 0) {
        fwrite(buf, 1, n, fout);
    }
    fclose(fstub);
    
    if (g_debug_mode) {
        printf("[DEBUG] Appending BPP payload from '%s'...\n", basic_path);
    }
    FILE *fbpp = fopen(basic_path, "rb");
    if (!fbpp) {
        fclose(fout);
        fprintf(stderr, "Error: Could not read temporary BPP payload '%s'\n", basic_path);
        return false;
    }
    
    uint32_t payload_size = 0;
    while ((n = fread(buf, 1, sizeof(buf), fbpp)) > 0) {
        fwrite(buf, 1, n, fout);
        payload_size += (uint32_t)n;
    }
    fclose(fbpp);
    
    if (g_debug_mode) {
        printf("[DEBUG] Appending 8-byte footer (payload size = %u bytes, tag = 'BPPE')...\n", (unsigned int)payload_size);
    }
    unsigned char footer[8];
    footer[0] = (unsigned char)(payload_size & 0xFF);
    footer[1] = (unsigned char)((payload_size >> 8) & 0xFF);
    footer[2] = (unsigned char)((payload_size >> 16) & 0xFF);
    footer[3] = (unsigned char)((payload_size >> 24) & 0xFF);
    
    footer[4] = 'B';
    footer[5] = 'P';
    footer[6] = 'P';
    footer[7] = 'E';
    
    fwrite(footer, 1, 8, fout);
    fclose(fout);
    
#ifndef _WIN32
    if (g_debug_mode) {
        printf("[DEBUG] Marking output binary as executable on Linux...\n");
    }
    char chmod_cmd[1024];
    snprintf(chmod_cmd, sizeof(chmod_cmd), "chmod +x \"%s\"", out_exe_path);
    int res = system(chmod_cmd);
    (void)res;
#endif

    if (g_debug_mode) {
        printf("[DEBUG] Packaging completed successfully.\n");
    }
    return true;
}



int main(int argc, char **argv) {
    if (argc < 2) {
        printf("BASIC++ Compiler (bppc) - v%s\n\n", BASIC_VERSION_STRING);
        printf("Usage:\n");
        printf("  bppc <input.bas> <output.bpp>                         (Compile to Bytecode)\n");
        printf("  bppc --standalone <input.bas> <output.exe>            (Compile Standalone Binary for Host OS)\n");
        printf("  bppc --standalone --windows <input.bas> <output.exe>  (Compile Standalone Binary for Windows)\n");
        printf("  bppc --standalone --linux <input.bas> <output>        (Compile Standalone Binary for Linux)\n");
        printf("\n");
        printf("Switches:\n");
        printf("  --help, -h, -?   Display this command usage information.\n");
        printf("  --about          Display information about the bppc compiler.\n");
        printf("  --license        Display the software license details.\n");
        printf("  --version        Display compiler version details.\n");
        printf("  --debug          Output verbose diagnostics during the compilation process.\n");
        return 0;
    }



    bool is_standalone = false;
    bool target_windows = false;
    bool target_linux = false;
    const char *infile = NULL;
    const char *outfile = NULL;

    int arg_idx = 1;
    while (arg_idx < argc) {
        if (strcmp(argv[arg_idx], "--help") == 0 || strcmp(argv[arg_idx], "-h") == 0 || strcmp(argv[arg_idx], "-?") == 0) {
            printf("BASIC++ Compiler (bppc) - v%s\n\n", BASIC_VERSION_STRING);
            printf("Usage:\n");
            printf("  bppc <input.bas> <output.bpp>                         (Compile to Bytecode)\n");
            printf("  bppc --standalone <input.bas> <output.exe>            (Compile Standalone Binary for Host OS)\n");
            printf("  bppc --standalone --windows <input.bas> <output.exe>  (Compile Standalone Binary for Windows)\n");
            printf("  bppc --standalone --linux <input.bas> <output>        (Compile Standalone Binary for Linux)\n");
            printf("\n");
            printf("Switches:\n");
            printf("  --help, -h, -?   Display this command usage information.\n");
            printf("  --about          Display information about the bppc compiler.\n");
            printf("  --license        Display the software license details.\n");
            printf("  --version        Display compiler version details.\n");
            printf("  --debug          Output verbose diagnostics during the compilation process.\n");
            return 0;
        } else if (strcmp(argv[arg_idx], "--about") == 0) {
            printf("BASIC++ Compiler (bppc)\n");
            printf("Compiles BASIC++ source scripts (.bas) into portable bytecode (.bpp) payloads\n");
            printf("and builds standalone cross-platform executable binaries for Windows and Linux.\n");
            return 0;
        } else if (strcmp(argv[arg_idx], "--license") == 0) {
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
        } else if (strcmp(argv[arg_idx], "--version") == 0) {
            printf("bppc version %s (synchronized with BASIC++ %s)\n", BASIC_VERSION_STRING, BASIC_VERSION_STRING);
            return 0;
        } else if (strcmp(argv[arg_idx], "--debug") == 0) {
            g_debug_mode = true;
        } else if (strcmp(argv[arg_idx], "--standalone") == 0) {
            is_standalone = true;
        } else if (strcmp(argv[arg_idx], "--windows") == 0) {
            target_windows = true;
            is_standalone = true;
        } else if (strcmp(argv[arg_idx], "--linux") == 0) {
            target_linux = true;
            is_standalone = true;
        } else {
            if (!infile) {
                infile = argv[arg_idx];
            } else if (!outfile) {
                outfile = argv[arg_idx];
            }
        }
        arg_idx++;
    }

    if (!infile || !outfile) {
        fprintf(stderr, "Error: Input and Output file parameters are required.\n");
        return 1;
    }

    #ifdef _WIN32
    if (target_linux) {
        target_windows = true;
    } else if (is_standalone && !target_windows) {
        target_windows = true;
    }
    #else
    if (target_windows) {
        target_linux = true;
    } else if (is_standalone && !target_linux) {
        target_linux = true;
    }
    #endif

    if (g_debug_mode) {
        printf("[DEBUG] Target Windows Standalone: %s\n", target_windows ? "YES" : "NO");
        printf("[DEBUG] Target Linux Standalone: %s\n", target_linux ? "YES" : "NO");
        printf("[DEBUG] Opening input source file: '%s'\n", infile);
    }

    FILE *in_f = fopen(infile, "rb");
    if (!in_f) {
        fprintf(stderr, "Error: Could not open input file '%s'\n", infile);
        return 1;
    }
    fseek(in_f, 0, SEEK_END);
    long in_size = ftell(in_f);
    fseek(in_f, 0, SEEK_SET);

    if (g_debug_mode) {
        printf("[DEBUG] Input file size: %ld bytes\n", in_size);
    }

    unsigned char *in_data = (unsigned char *)calloc(1, in_size >= 0 ? in_size + 1 : 1);
    if (!in_data) {
        fclose(in_f);
        fprintf(stderr, "Error: Out of memory loading input file\n");
        return 1;
    }
    if (in_size > 0) {
        if (fread(in_data, 1, in_size, in_f) != (size_t)in_size) {
            fclose(in_f);
            free(in_data);
            fprintf(stderr, "Error: Failed to read input file\n");
            return 1;
        }
    }
    fclose(in_f);

    char *source_text = NULL;
    bool is_detokenized = false;
    if (in_size > 0 && in_data[0] == 0xFF) {
        if (g_debug_mode) {
            printf("[DEBUG] Detected tokenized GW-BASIC binary format. Decoding...\n");
        }
        source_text = detokenize_gw_basic(in_data, in_size);
        free(in_data);
        in_data = NULL;
        if (!source_text) {
            fprintf(stderr, "Error: Failed to detokenize GW-BASIC program\n");
            return 1;
        }
        is_detokenized = true;
    } else {
        in_data[in_size] = '\0';
        source_text = (char *)in_data;
    }

    if (is_standalone) {
        char temp_bpp[1024];
        snprintf(temp_bpp, sizeof(temp_bpp), "%s.tmp.bpp", outfile);
        
        if (!compile_to_bpp(source_text, temp_bpp)) {
            if (is_detokenized) free(source_text);
            fprintf(stderr, "Error: Failed to compile source code to intermediate bytecode\n");
            return 1;
        }
        
        if (target_windows && target_linux) {
            char base_path[1024];
            strncpy(base_path, outfile, sizeof(base_path) - 1);
            base_path[sizeof(base_path) - 1] = '\0';
            
            size_t out_len = strlen(base_path);
            if (out_len > 4 && strcasecmp(base_path + out_len - 4, ".exe") == 0) {
                base_path[out_len - 4] = '\0';
            }
            
            char win_path[1024];
            char linux_path[1024];
            snprintf(win_path, sizeof(win_path), "%s.exe", base_path);
            snprintf(linux_path, sizeof(linux_path), "%s", base_path);
            
            if (!package_standalone(temp_bpp, win_path, argv[0], "basstub.exe")) {
                remove(temp_bpp);
                if (is_detokenized) free(source_text);
                return 1;
            }
            if (!package_standalone(temp_bpp, linux_path, argv[0], "basstub")) {
                remove(temp_bpp);
                if (is_detokenized) free(source_text);
                return 1;
            }
            printf("Successfully compiled standalone Windows executable '%s' and Linux executable '%s'\n", win_path, linux_path);
        } else if (target_windows) {
            char win_path[1024];
            size_t out_len = strlen(outfile);
            if (out_len > 4 && strcasecmp(outfile + out_len - 4, ".exe") == 0) {
                strncpy(win_path, outfile, sizeof(win_path) - 1);
                win_path[sizeof(win_path) - 1] = '\0';
            } else {
                snprintf(win_path, sizeof(win_path), "%s.exe", outfile);
            }
            
            if (!package_standalone(temp_bpp, win_path, argv[0], "basstub.exe")) {
                remove(temp_bpp);
                if (is_detokenized) free(source_text);
                return 1;
            }
            printf("Successfully compiled standalone Windows executable '%s'\n", win_path);
        } else if (target_linux) {
            char linux_path[1024];
            strncpy(linux_path, outfile, sizeof(linux_path) - 1);
            linux_path[sizeof(linux_path) - 1] = '\0';
            size_t out_len = strlen(linux_path);
            if (out_len > 4 && strcasecmp(linux_path + out_len - 4, ".exe") == 0) {
                linux_path[out_len - 4] = '\0';
            }
            
            if (!package_standalone(temp_bpp, linux_path, argv[0], "basstub")) {
                remove(temp_bpp);
                if (is_detokenized) free(source_text);
                return 1;
            }
            printf("Successfully compiled standalone Linux executable '%s'\n", linux_path);
        }
        
        remove(temp_bpp);
    } else {
        if (!compile_to_bpp(source_text, outfile)) {
            if (is_detokenized) free(source_text);
            fprintf(stderr, "Error: Failed to compile source code to BPP bytecode '%s'\n", outfile);
            return 1;
        }
        printf("Successfully compiled bytecode '%s'\n", outfile);
    }

    if (is_detokenized) {
        free(source_text);
    }
    return 0;
}


