// FILENAME: bppc_bytecode.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (bppc_internal.h)
// Implements toolchain and compiler subsystem components for bppc_bytecode.
//
// ---- Includes ----

#include "tools/bppc_internal.h"
#include "platform/platform.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "runtime/memory/alloc.h"

bool g_bppc_debug_mode = false;

typedef struct {
    uint16_t line_num;
    char *text;
} CompiledLine;

void bppc_write_le16(unsigned char *buf, uint16_t val) {
    buf[0] = (unsigned char)(val & 0xFF);
    buf[1] = (unsigned char)((val >> 8) & 0xFF);
}

static void quicksort_compiled_lines(CompiledLine *arr, int low, int high) {
    if (low < high) {
        uint16_t pivot = arr[high].line_num;
        int i = low - 1;
        for (int j = low; j < high; j++) {
            if (arr[j].line_num <= pivot) {
                i++;
                CompiledLine tmp = arr[i];
                arr[i] = arr[j];
                arr[j] = tmp;
            }
        }
        CompiledLine tmp = arr[i + 1];
        arr[i + 1] = arr[high];
        arr[high] = tmp;
        int pi = i + 1;
        if (pi > 0) quicksort_compiled_lines(arr, low, pi - 1);
        quicksort_compiled_lines(arr, pi + 1, high);
    }
}

bool compile_to_bpp(const char *source_text, const char *out_bpp_path) {
    if (g_bppc_debug_mode) {
        platform_console_printf("[DEBUG] Starting bytecode compilation to: '%s'\n", out_bpp_path);
    }
    size_t line_cap = 256;
    size_t line_count = 0;
    CompiledLine *lines = (CompiledLine *)runtime_calloc(line_cap, sizeof(CompiledLine));
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
        
        char *trimmed = bppc_trim(line_buf);
        if (trimmed[0] == '\0') continue;
        
        uint16_t line_num = 0;
        const char *stmt_text = NULL;
        int lnum_parsed = 0;
        if (bppc_parse_line_number(trimmed, &lnum_parsed, &stmt_text)) {
            line_num = (uint16_t)lnum_parsed;
            char temp[2048];
            runtime_strncpy(temp, stmt_text, sizeof(temp) - 1);
            temp[sizeof(temp) - 1] = '\0';
            trimmed = bppc_trim(temp);
        } else {
            line_num = auto_line;
            auto_line += 10;
        }
        
        if (g_bppc_debug_mode) {
            platform_console_printf("[DEBUG] Parsed line %d: \"%s\"\n", (int)line_num, trimmed);
        }
        
        if (line_count >= line_cap) {
            line_cap *= 2;
            CompiledLine *new_lines = runtime_realloc(lines, line_cap * sizeof(CompiledLine));
            if (!new_lines) {
                for (size_t i = 0; i < line_count; ++i) runtime_free(lines[i].text);
                runtime_free(lines);
                return false;
            }
            lines = new_lines;
        }
        
        lines[line_count].line_num = line_num;
        lines[line_count].text = bppc_strdup(trimmed);
        line_count++;
    }
    
    if (g_bppc_debug_mode) {
        platform_console_printf("[DEBUG] Sorting %d compiled lines by line number...\n", (int)line_count);
    }
    if (line_count > 1) {
        quicksort_compiled_lines(lines, 0, (int)line_count - 1);
    }
    
    if (g_bppc_debug_mode) {
        platform_console_printf("[DEBUG] Writing bytecode output to '%s'...\n", out_bpp_path);
    }
    void *fout = platform_file_open(out_bpp_path, "wb");
    if (!fout) {
        for (size_t i = 0; i < line_count; ++i) runtime_free(lines[i].text);
        runtime_free(lines);
        return false;
    }
    
    unsigned char header[16];
    runtime_memset(header, 0, sizeof(header));
    header[0] = 'B';
    header[1] = 'P';
    header[2] = 'P';
    header[3] = 0x1A;
    header[4] = 1;
    header[5] = 2;
    bppc_write_le16(&header[8], (uint16_t)line_count);
    
    if (g_bppc_debug_mode) {
        platform_console_printf("[DEBUG] Writing 16-byte BPP file header...\n");
    }
    platform_file_write(fout, header, 16);
    
    for (size_t i = 0; i < line_count; ++i) {
        uint16_t text_len = (uint16_t)runtime_strlen(lines[i].text);
        unsigned char rec[4];
        bppc_write_le16(&rec[0], lines[i].line_num);
        bppc_write_le16(&rec[2], text_len);
        if (g_bppc_debug_mode) {
            platform_console_printf("[DEBUG] Writing record: line=%d, len=%d, text=\"%s\"\n", (int)lines[i].line_num, (int)text_len, lines[i].text);
        }
        platform_file_write(fout, rec, 4);
        if (text_len > 0) {
            platform_file_write(fout, lines[i].text, text_len);
        }
    }
    
    platform_file_close(fout);
    for (size_t i = 0; i < line_count; ++i) runtime_free(lines[i].text);
    runtime_free(lines);
    if (g_bppc_debug_mode) {
        platform_console_printf("[DEBUG] Bytecode compilation completed successfully.\n");
    }
    return true;
}

bool package_standalone(const char *basic_path, const char *out_exe_path, const char *argv0, const char *stub_filename) {
    if (g_bppc_debug_mode) {
        platform_console_printf("[DEBUG] Packaging standalone binary: '%s' using stub: '%s'\n", out_exe_path, stub_filename);
    }
    char stub_path[1024] = "";
    const char *last_slash = runtime_strrchr(argv0, '/');
    if (!last_slash) last_slash = runtime_strrchr(argv0, '\\');
    
    if (last_slash) {
        size_t dir_len = (size_t)(last_slash - argv0 + 1);
        runtime_memcpy(stub_path, argv0, dir_len);
        stub_path[dir_len] = '\0';
        runtime_snprintf(stub_path + dir_len, sizeof(stub_path) - dir_len, "%s", stub_filename);
    } else {
        runtime_snprintf(stub_path, sizeof(stub_path), "%s", stub_filename);
    }
    
    if (g_bppc_debug_mode) {
        platform_console_printf("[DEBUG] Attempting to open stub at path: '%s'\n", stub_path);
    }
    void *fstub = platform_file_open(stub_path, "rb");
    if (!fstub) {
        fstub = platform_file_open(stub_filename, "rb");
        if (!fstub) {
            platform_console_eprintf("Error: Could not locate '%s' runner stub (expected in '%s' or current directory)\n", stub_filename, stub_path);
            return false;
        }
    }
    
    void *fout = platform_file_open(out_exe_path, "wb");
    if (!fout) {
        platform_file_close(fstub);
        platform_console_eprintf("Error: Could not open output executable '%s' for writing\n", out_exe_path);
        return false;
    }
    
    if (g_bppc_debug_mode) {
        platform_console_printf("[DEBUG] Copying stub to output binary...\n");
    }
    char buf[4096];
    size_t n;
    while ((n = platform_file_read(fstub, buf, sizeof(buf))) > 0) {
        platform_file_write(fout, buf, n);
    }
    platform_file_close(fstub);
    
    if (g_bppc_debug_mode) {
        platform_console_printf("[DEBUG] Appending BPP payload from '%s'...\n", basic_path);
    }
    void *fbpp = platform_file_open(basic_path, "rb");
    if (!fbpp) {
        platform_file_close(fout);
        platform_console_eprintf("Error: Could not read temporary BPP payload '%s'\n", basic_path);
        return false;
    }
    
    uint32_t payload_size = 0;
    while ((n = platform_file_read(fbpp, buf, sizeof(buf))) > 0) {
        platform_file_write(fout, buf, n);
        payload_size += (uint32_t)n;
    }
    platform_file_close(fbpp);
    
    if (g_bppc_debug_mode) {
        platform_console_printf("[DEBUG] Appending 8-byte footer (payload size = %u bytes, tag = 'BPPE')...\n", (unsigned int)payload_size);
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
    
    platform_file_write(fout, footer, 8);
    platform_file_close(fout);
    
#ifndef _WIN32
    if (g_bppc_debug_mode) {
        platform_console_printf("[DEBUG] Marking output binary as executable on Linux...\n");
    }
    char chmod_cmd[1024];
    runtime_snprintf(chmod_cmd, sizeof(chmod_cmd), "chmod +x \"%s\"", out_exe_path);
    platform_execute_command(chmod_cmd);
#endif

    if (g_bppc_debug_mode) {
        platform_console_printf("[DEBUG] Packaging completed successfully.\n");
    }
    return true;
}
