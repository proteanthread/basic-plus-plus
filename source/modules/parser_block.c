 // ---
 // BASIC++ Interpreter - parser_block.c
 // ---
 //
 // Structured block parser implementation.
 //
 // IMPLEMENTATION:
 // Two-pass loading:
 //
 // Pass 1 (already done by lib_space_load_source):
 //   Scan source for SUB/FUNCTION/DEF FN declarations
 //   and register symbol stubs in the LoadedLibrary.
 //
 // Pass 2 (this module):
 //   Read source line by line.
 //   Store each line in the library's src_lines table.
 //   When entering a SUB/FUNCTION block, record the entry
 //   point (source line index where the block body starts).
 //   When exiting (END SUB/END FUNCTION), record the end.
 //
 // EXECUTION MODEL:
 //   Library code runs through the interpreter's native
 //   parser_execute_line() a" NOT the PCode compiler.
 //   This gives full access to all BASIC constructs:
 //   LOCAL, EXIT SUB, ELSEIF, CALL, named variables, etc.
 //
 // BLOCK STATE MACHINE:
 //   OUTSIDE   -> SUB/FUNCTION keyword   -> INSIDE_BLOCK
 //   INSIDE    -> END SUB/END FUNCTION   -> OUTSIDE
 //   OUTSIDE   -> DEF FN                 -> single-line
 //
 // C89/C90 COMPLIANT.
 //
 // ---

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "parser_block.h"
#include "../lexer.h"
#include "../errors.h"

// --- Block parse state ---
typedef enum {
    BLK_OUTSIDE = 0,
    BLK_IN_SUB,
    BLK_IN_FUNCTION
} BlockState;

// --- Case-insensitive keyword check ---
static int blk_kw_match(const char *line, const char *kw)
{
    const char *p = line;
    // Skip leading whitespace
    while (*p == ' ' || *p == '\t') p++;
    // Skip optional line number
    while (*p >= '0' && *p <= '9') p++;
    while (*p == ' ') p++;
    // Match keyword
    while (*kw) {
        if (toupper((unsigned char)*p) !=
            toupper((unsigned char)*kw)) return 0;
        p++; kw++;
    }
    // Must be followed by space, tab, '(', EOL, or NUL
    if (*p != ' ' && *p != '\t' && *p != '(' &&
        *p != '\0' && *p != '\r' && *p != '\n')
        return 0;
    return 1;
}

// --- Check for END SUB / END FUNCTION ---
static int blk_is_end_sub(const char *line)
{
    const char *p = line;
    while (*p == ' ' || *p == '\t') p++;
    while (*p >= '0' && *p <= '9') p++;
    while (*p == ' ') p++;
    if (toupper((unsigned char)p[0]) != 'E' ||
        toupper((unsigned char)p[1]) != 'N' ||
        toupper((unsigned char)p[2]) != 'D') return 0;
    p += 3;
    while (*p == ' ') p++;
    if (toupper((unsigned char)p[0]) == 'S' &&
        toupper((unsigned char)p[1]) == 'U' &&
        toupper((unsigned char)p[2]) == 'B')
        return 1;
    return 0;
}

static int blk_is_end_function(const char *line)
{
    const char *p = line;
    while (*p == ' ' || *p == '\t') p++;
    while (*p >= '0' && *p <= '9') p++;
    while (*p == ' ') p++;
    if (toupper((unsigned char)p[0]) != 'E' ||
        toupper((unsigned char)p[1]) != 'N' ||
        toupper((unsigned char)p[2]) != 'D') return 0;
    p += 3;
    while (*p == ' ') p++;
    if (toupper((unsigned char)p[0]) == 'F' &&
        toupper((unsigned char)p[1]) == 'U' &&
        toupper((unsigned char)p[2]) == 'N' &&
        toupper((unsigned char)p[3]) == 'C' &&
        toupper((unsigned char)p[4]) == 'T' &&
        toupper((unsigned char)p[5]) == 'I' &&
        toupper((unsigned char)p[6]) == 'O' &&
        toupper((unsigned char)p[7]) == 'N')
        return 1;
    return 0;
}

// --- Extract block name from SUB/FUNCTION line ---
 // "SUB TURTLE_INIT(X, Y)" -> "TURTLE_INIT"
static int blk_extract_name(const char *line, const char *kw,
                             char *buf, int buflen)
{
    const char *p = line;
    int ni = 0;
    while (*p == ' ' || *p == '\t') p++;
    while (*p >= '0' && *p <= '9') p++;
    while (*p == ' ') p++;
    while (*kw) { p++; kw++; }
    while (*p == ' ') p++;
    while (*p && *p != '(' && *p != ' ' && *p != '\n' &&
           *p != '\r' && ni < buflen - 1) {
        buf[ni++] = (char)toupper((unsigned char)*p);
        p++;
    }
    buf[ni] = '\0';
    return ni;
}

// --- Count parameters ---
static int blk_count_params(const char *line)
{
    const char *p = strchr(line, '(');
    int count = 0;
    if (!p) return 0;
    p++;
    while (*p == ' ') p++;
    if (*p == ')') return 0;
    count = 1;
    while (*p && *p != ')') {
        if (*p == ',') count++;
        p++;
    }
    return count;
}

// --- Find symbol by name in library ---
static LibSymbol *blk_find_symbol(LoadedLibrary *lib,
                                   const char *name)
{
    for (int i = 0; i < lib->symbol_count; i++) {
        if (lib->symbols[i].active) {
            const char *a = lib->symbols[i].name;
            const char *b = name;
            int match = 1;
            while (*a && *b) {
                if (toupper((unsigned char)*a) !=
                    toupper((unsigned char)*b)) {
                    match = 0; break;
                }
                a++; b++;
            }
            if (match && *a == '\0' && *b == '\0')
                return &lib->symbols[i];
        }
    }
    return NULL;
}

// --- Add a source line to the library ---
static int blk_add_line(LoadedLibrary *lib,
                          const char *text, int vline)
{
    LibSourceLine *sl;

    // Grow if needed
    if (lib->src_line_count >= lib->src_line_cap) {
        int new_cap = lib->src_line_cap == 0
                        ? 128 : lib->src_line_cap * 2;
        LibSourceLine *new_buf = (LibSourceLine *)realloc(
            lib->src_lines,
            (size_t)new_cap * sizeof(LibSourceLine));
        if (!new_buf) return -1;
        lib->src_lines = new_buf;
        lib->src_line_cap = new_cap;
    }

    sl = &lib->src_lines[lib->src_line_count];
    strncpy(sl->text, text, 255);
    sl->text[255] = '\0';
    sl->vline = vline;
    lib->src_line_count++;
    return lib->src_line_count - 1; // return index
}

// ===================================================================
 // PUBLIC API
 // ===================================================================

int parser_block_compile_line(const char *line, int vline,
                              LoadedLibrary *lib)
{
    // In interpreter mode, store the line for later execution
    return blk_add_line(lib, line, vline);
}

int parser_block_compile_file(const char *path,
                              LoadedLibrary *lib,
                              BlockParseResult *result)
{
    FILE *fp;
    char line[512];
    int vline;
    BlockState state;
    int block_start_idx;
    char block_name[MAX_LIB_NAME];
    int total_symbols;
    int total_lines;

    if (!path || !lib || !result) return -1;

    memset(result, 0, sizeof(BlockParseResult));

    // Initialize source line storage
    lib->src_lines = NULL;
    lib->src_line_count = 0;
    lib->src_line_cap = 0;

    fp = fopen(path, "r");
    if (!fp) {
        printf("Cannot open: %s\n", path);
        return -1;
    }

    vline = LIB_VLINE_BASE;
    state = BLK_OUTSIDE;
    block_start_idx = 0;
    block_name[0] = '\0';
    total_symbols = 0;
    total_lines = 0;

    while (fgets(line, sizeof(line), fp)) {
        total_lines++;

        // Trim trailing newline
        {
            int len = (int)strlen(line);
            while (len > 0 && (line[len-1] == '\n' ||
                               line[len-1] == '\r'))
                line[--len] = '\0';
        }

        // Skip empty lines
        {
            const char *p = line;
            while (*p == ' ' || *p == '\t') p++;
            if (*p == '\0') { vline++; continue; }
        }

        // Skip REM @header lines (metadata already parsed)
        {
            const char *p = line;
            while (*p == ' ' || *p == '\t') p++;
            if (toupper((unsigned char)p[0]) == 'R' &&
                toupper((unsigned char)p[1]) == 'E' &&
                toupper((unsigned char)p[2]) == 'M') {
                const char *r = p + 3;
                while (*r == ' ') r++;
                if (*r == '@') { vline++; continue; }
                // Store REM inside blocks for completeness
                if (state != BLK_OUTSIDE) {
                    blk_add_line(lib, line, vline);
                }
                vline++;
                continue;
            }
        }

        // Check for block start: SUB
        if (state == BLK_OUTSIDE &&
            blk_kw_match(line, "SUB")) {
            // Store the SUB declaration line
            block_start_idx = blk_add_line(lib, line, vline);
            state = BLK_IN_SUB;
            blk_extract_name(line, "SUB", block_name,
                            MAX_LIB_NAME);

            // Patch symbol: entry_offset = source line index
            {
                LibSymbol *sym = blk_find_symbol(lib,
                                                  block_name);
                if (sym) {
                    sym->entry_offset = block_start_idx;
                    sym->param_count =
                        blk_count_params(line);
                }
            }
            total_symbols++;
            vline++;
            continue;
        }

        // Check for block start: FUNCTION
        if (state == BLK_OUTSIDE &&
            blk_kw_match(line, "FUNCTION")) {
            block_start_idx = blk_add_line(lib, line, vline);
            state = BLK_IN_FUNCTION;
            blk_extract_name(line, "FUNCTION", block_name,
                            MAX_LIB_NAME);

            {
                LibSymbol *sym = blk_find_symbol(lib,
                                                  block_name);
                if (sym) {
                    sym->entry_offset = block_start_idx;
                    sym->param_count =
                        blk_count_params(line);
                }
            }
            total_symbols++;
            vline++;
            continue;
        }

        // Check for block start: DEF FN (single-line)
        if (state == BLK_OUTSIDE &&
            blk_kw_match(line, "DEF")) {
            int def_idx = blk_add_line(lib, line, vline);

            // Find and patch the symbol
            {
                const char *dp = line;
                char fn_name[MAX_LIB_NAME];
                int ni = 0;
                LibSymbol *sym;
                while (*dp == ' ' || *dp == '\t') dp++;
                // Skip past "DEF FN"
                while (*dp && toupper((unsigned char)*dp)
                       != 'N') dp++;
                if (*dp) dp++; // skip N
                while (*dp == ' ') dp++;
                while (*dp && *dp != '(' && *dp != ' ' &&
                       *dp != '=' && *dp != '\n' &&
                       ni < MAX_LIB_NAME - 1) {
                    fn_name[ni++] = (char)toupper(
                        (unsigned char)*dp);
                    dp++;
                }
                fn_name[ni] = '\0';
                sym = blk_find_symbol(lib, fn_name);
                if (sym) {
                    sym->entry_offset = def_idx;
                }
            }
            total_symbols++;
            vline++;
            continue;
        }

        // Check for block end: END SUB
        if (state == BLK_IN_SUB && blk_is_end_sub(line)) {
            blk_add_line(lib, line, vline);
            state = BLK_OUTSIDE;
            block_name[0] = '\0';
            vline++;
            continue;
        }

        // Check for block end: END FUNCTION
        if (state == BLK_IN_FUNCTION &&
            blk_is_end_function(line)) {
            blk_add_line(lib, line, vline);
            state = BLK_OUTSIDE;
            block_name[0] = '\0';
            vline++;
            continue;
        }

        // Inside a block: store the line
        if (state == BLK_IN_SUB || state == BLK_IN_FUNCTION) {
            blk_add_line(lib, line, vline);
        }

        // Outside blocks: module-level code
        if (state == BLK_OUTSIDE) {
            const char *p = line;
            while (*p == ' ' || *p == '\t') p++;
            // Skip standalone REM
            if (toupper((unsigned char)p[0]) == 'R' &&
                toupper((unsigned char)p[1]) == 'E' &&
                toupper((unsigned char)p[2]) == 'M') {
                vline++;
                continue;
            }
            // Module-level init code
            blk_add_line(lib, line, vline);
        }

        vline++;
    }

    // Check for unterminated blocks
    if (state != BLK_OUTSIDE) {
        printf("  Warning: unterminated %s block '%s'\n",
               state == BLK_IN_SUB ? "SUB" : "FUNCTION",
               block_name);
    }

    fclose(fp);

    // Mark as compiled (source stored, ready for execution)
    lib->compiled = 1;

    // Fill result
    result->symbols_found = total_symbols;
    result->instructions = lib->src_line_count;
    result->errors = 0;
    result->lines_parsed = total_lines;

    return 0;
}
