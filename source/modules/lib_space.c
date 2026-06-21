/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: lib_space.c
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
 // BASIC++ Interpreter - lib_space.c
 // ---
 //
 // Library Program Space manager implementation.
 //
 // IMPLEMENTATION:
 // Static array of MAX_LOADED_LIBS LoadedLibrary slots.
 // Libraries are loaded by:
 //   1. Security validation (SECOP_EXT_LOAD, path, pinning)
 //   2. Source parsing OR .BPL deserialization
 //   3. Bytecode compilation (source mode)
 //   4. Symbol registration
 //   5. Variable space initialization
 //
 // Invocation pushes a separate VM execution context,
 // runs library bytecode, and restores the caller's state.
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
#include <ctype.h>
#include <stdlib.h>
#include "lib_space.h"
#include "parser_block.h"
#include "../runtime.h"
#include "../lexer.h"
#include "../errors.h"
#include "../scope_stack.h"
#include "../dialect.h"

// Interpreter entry point for executing a line
extern void parser_execute_line(Lexer *lex, RuntimeState *rt,
                                int line_num);
#include "../security.h"
#include <stdbool.h>

// Forward declarations
int lib_space_load_bpl(const char *path);
int bpl_load(const char *filename, LoadedLibrary *lib);

// --- Library Table ---
static LoadedLibrary lib_slots[MAX_LOADED_LIBS];
static int lib_slot_count = 0;

// --- Case-insensitive string compare (C89) ---
static int ls_str_iequal(const char *a, const char *b)
{
    if (!a || !b) return 0;
    while (*a && *b) {
        if (toupper((unsigned char)*a) !=
            toupper((unsigned char)*b))
            return 0;
        a++; b++;
    }
    return (*a == '\0' && *b == '\0');
}

// --- Trim trailing whitespace/newlines ---
static void ls_trim(char *s)
{
    char *end;
    if (!s || !*s) return;
    end = s + strlen(s) - 1;
    while (end >= s && (*end == ' ' || *end == '\n' ||
                        *end == '\r' || *end == '\t'))
        *end-- = '\0';
}

// --- Parse REM @TAG VALUE from a line ---
 // Returns pointer to value after tag, or NULL.
static const char *ls_parse_rem_tag(const char *line,
                                     const char *tag)
{
    const char *p = line;
    // Skip leading whitespace
    while (*p == ' ' || *p == '\t') p++;
    // Skip optional line number
    while (*p >= '0' && *p <= '9') p++;
    while (*p == ' ') p++;
    // Check for REM
    if (toupper((unsigned char)p[0]) != 'R' ||
        toupper((unsigned char)p[1]) != 'E' ||
        toupper((unsigned char)p[2]) != 'M') return NULL;
    p += 3;
    while (*p == ' ') p++;
    if (*p != '@') return NULL;
    p++;
    // Check tag name
    while (*tag) {
        if (toupper((unsigned char)*p) !=
            toupper((unsigned char)*tag)) return NULL;
        p++; tag++;
    }
    while (*p == ' ') p++;
    return p;
}

// --- Find a free slot ---
static int ls_find_free_slot(void)
{
    for (int i = 0; i < lib_slot_count; i++) {
        if (!lib_slots[i].loaded) return i;
    }
    if (lib_slot_count < MAX_LOADED_LIBS)
        return lib_slot_count;
    return -1;
}

// ===================================================================
 // PUBLIC API
 // ===================================================================

void lib_space_init(void)
{
    memset(lib_slots, 0, sizeof(lib_slots));
    lib_slot_count = 0;
}

void lib_space_shutdown(void)
{
    for (int i = 0; i < lib_slot_count; i++) {
        if (lib_slots[i].loaded) {
            // Free compiled bytecode
            if (lib_slots[i].compiled) {
                pcode_free(&lib_slots[i].pcode);
            }
            // Free source line buffer
            if (lib_slots[i].src_lines) {
                free(lib_slots[i].src_lines);
                lib_slots[i].src_lines = NULL;
            }
            // Free string variables
            {
                for (int j = 0; j < 26; j++) {
                    if (lib_slots[i].str_vars[j]) {
                        free(lib_slots[i].str_vars[j]);
                        lib_slots[i].str_vars[j] = NULL;
                    }
                }
            }
            lib_slots[i].loaded = 0;
        }
    }
    lib_slot_count = 0;
}

int lib_space_load_source(const char *path, LibExtType ext_type,
                          void *rt)
{
    FILE *fp;
    char line[512];
    int slot;
    const char *val;
    int header_lines;
    LoadedLibrary *lib;

    (void)rt;

    // Security gate
    if (security_check(SECOP_EXT_LOAD, 0) != 0) return -1;
    if (security_check_path(path, 0) != 0) return -1;

    // Find slot
    slot = ls_find_free_slot();
    if (slot < 0) {
        printf("Library space full (max %d).\n",
               MAX_LOADED_LIBS);
        return -1;
    }

    // Initialize slot
    lib = &lib_slots[slot];
    memset(lib, 0, sizeof(LoadedLibrary));
    strncpy(lib->path, path, 255);
    lib->ext_type = ext_type;
    lib->required_level = SEC_COUNT; // unpinned

    // Open source file
    fp = fopen(path, "r");
    if (!fp) {
        printf("Cannot open: %s\n", path);
        return -1;
    }

    // --- Pass 1: Scan REM @headers (first 20 lines) ---
    header_lines = 0;
    while (header_lines < 20 && fgets(line, sizeof(line), fp)) {
        header_lines++;

        val = ls_parse_rem_tag(line, "LIBRARY");
        if (val) {
            strncpy(lib->name, val, MAX_LIB_NAME - 1);
            lib->name[MAX_LIB_NAME - 1] = '\0';
            ls_trim(lib->name);
            continue;
        }

        val = ls_parse_rem_tag(line, "VERSION");
        if (val) {
            strncpy(lib->version, val, 15);
            lib->version[15] = '\0';
            ls_trim(lib->version);
            continue;
        }

        val = ls_parse_rem_tag(line, "SECURITY");
        if (val) {
            char lvl_name[32];
            int lvl;
            strncpy(lvl_name, val, 31);
            lvl_name[31] = '\0';
            ls_trim(lvl_name);
            lvl = security_find_level_by_name(lvl_name);
            if (lvl >= 0) {
                lib->required_level = (SecLevel)lvl;
            }
            continue;
        }
    }

    // If no name found, derive from filename
    if (lib->name[0] == '\0') {
        const char *base = path;
        const char *p = path;
        while (*p) {
            if (*p == '/' || *p == '\\') base = p + 1;
            p++;
        }
        strncpy(lib->name, base, MAX_LIB_NAME - 1);
        lib->name[MAX_LIB_NAME - 1] = '\0';
        // Remove extension
        {
            char *dot = strrchr(lib->name, '.');
            if (dot) *dot = '\0';
        }
        // Uppercase
        for (int i = 0; lib->name[i]; i++)
            lib->name[i] = (char)toupper(
                (unsigned char)lib->name[i]);
    }

    // Check security pinning
    if (!security_check_pinned_level(lib->required_level)) {
        printf("Library '%s' requires security level %s "
               "(current: %s).\n",
               lib->name,
               security_level_name(lib->required_level),
               security_level_name(security_get_level()));
        fclose(fp);
        return -1;
    }

    // Check for duplicate
    {
        for (int i = 0; i < lib_slot_count; i++) {
            if (i != slot && lib_slots[i].loaded &&
                ls_str_iequal(lib_slots[i].name, lib->name)) {
                printf("Library '%s' already loaded.\n",
                       lib->name);
                fclose(fp);
                return -1;
            }
        }
    }

    // --- Pass 2: Parse SUB/FUNCTION/DEF FN blocks ---
     //
     // TODO: This is the structured block parser integration
     // point. When parser_block.c is implemented, it will:
     //   1. Rewind to start of file
     //   2. Parse each SUB/FUNCTION/DEF FN block
     //   3. Compile each block to PCode
     //   4. Register symbols with entry point offsets
     //
     // For now, we register the library metadata and report
     // that structured parsing is pending.

    // Rewind for full parse
    rewind(fp);

    // Count SUB/FUNCTION definitions for progress
    {
        int sub_count = 0;
        int func_count = 0;
        int def_fn_count = 0;
        char *p;

        while (fgets(line, sizeof(line), fp)) {
            // Skip leading whitespace
            p = line;
            while (*p == ' ' || *p == '\t') p++;

            // Check for SUB keyword
            if (toupper((unsigned char)p[0]) == 'S' &&
                toupper((unsigned char)p[1]) == 'U' &&
                toupper((unsigned char)p[2]) == 'B' &&
                (p[3] == ' ' || p[3] == '\t')) {
                // Extract SUB name
                if (lib->symbol_count < MAX_LIB_SYMBOLS) {
                    LibSymbol *sym =
                        &lib->symbols[lib->symbol_count];
                    char *name_start = p + 4;
                    int ni = 0;
                    while (*name_start == ' ') name_start++;
                    while (name_start[ni] &&
                           name_start[ni] != '(' &&
                           name_start[ni] != ' ' &&
                           name_start[ni] != '\n' &&
                           name_start[ni] != '\r' &&
                           ni < MAX_LIB_NAME - 1) {
                        sym->name[ni] = (char)toupper(
                            (unsigned char)name_start[ni]);
                        ni++;
                    }
                    sym->name[ni] = '\0';
                    sym->type = LIB_SYM_SUB;
                    sym->entry_offset = -1; // TBD at compile
                    sym->param_count = 0; // TBD at compile
                    sym->active = 1;
                    lib->symbol_count++;
                    sub_count++;
                }
            }

            // Check for FUNCTION keyword
            if (toupper((unsigned char)p[0]) == 'F' &&
                toupper((unsigned char)p[1]) == 'U' &&
                toupper((unsigned char)p[2]) == 'N' &&
                toupper((unsigned char)p[3]) == 'C' &&
                toupper((unsigned char)p[4]) == 'T' &&
                toupper((unsigned char)p[5]) == 'I' &&
                toupper((unsigned char)p[6]) == 'O' &&
                toupper((unsigned char)p[7]) == 'N' &&
                (p[8] == ' ' || p[8] == '\t')) {
                if (lib->symbol_count < MAX_LIB_SYMBOLS) {
                    LibSymbol *sym =
                        &lib->symbols[lib->symbol_count];
                    char *name_start = p + 9;
                    int ni = 0;
                    while (*name_start == ' ') name_start++;
                    while (name_start[ni] &&
                           name_start[ni] != '(' &&
                           name_start[ni] != ' ' &&
                           name_start[ni] != '\n' &&
                           name_start[ni] != '\r' &&
                           ni < MAX_LIB_NAME - 1) {
                        sym->name[ni] = (char)toupper(
                            (unsigned char)name_start[ni]);
                        ni++;
                    }
                    sym->name[ni] = '\0';
                    sym->type = LIB_SYM_FUNCTION;
                    sym->entry_offset = -1;
                    sym->param_count = 0;
                    sym->active = 1;
                    lib->symbol_count++;
                    func_count++;
                }
            }

            // Check for DEF FN keyword
            if (toupper((unsigned char)p[0]) == 'D' &&
                toupper((unsigned char)p[1]) == 'E' &&
                toupper((unsigned char)p[2]) == 'F' &&
                p[3] == ' ') {
                char *dp = p + 4;
                while (*dp == ' ') dp++;
                if (toupper((unsigned char)dp[0]) == 'F' &&
                    toupper((unsigned char)dp[1]) == 'N' &&
                    (dp[2] == ' ' || dp[2] == '_' ||
                     (dp[2] >= 'A' && dp[2] <= 'Z') ||
                     (dp[2] >= 'a' && dp[2] <= 'z'))) {
                    if (lib->symbol_count < MAX_LIB_SYMBOLS) {
                        LibSymbol *sym =
                            &lib->symbols[lib->symbol_count];
                        // Skip "FN" and optional space
                        char *name_start = dp + 2;
                        int ni = 0;
                        while (*name_start == ' ') name_start++;
                        while (name_start[ni] &&
                               name_start[ni] != '(' &&
                               name_start[ni] != ' ' &&
                               name_start[ni] != '=' &&
                               name_start[ni] != '\n' &&
                               ni < MAX_LIB_NAME - 1) {
                            sym->name[ni] = (char)toupper(
                                (unsigned char)name_start[ni]);
                            ni++;
                        }
                        sym->name[ni] = '\0';
                        sym->type = LIB_SYM_DEF_FN;
                        sym->entry_offset = -1;
                        sym->param_count = 0;
                        sym->active = 1;
                        lib->symbol_count++;
                        def_fn_count++;
                    }
                }
            }
        }

        // Report
        lib->loaded = 1;
        if (slot >= lib_slot_count) lib_slot_count = slot + 1;

        printf("Loaded library: %s", lib->name);
        if (lib->version[0])
            printf(" v%s", lib->version);
        if (lib->required_level != SEC_COUNT)
            printf(" [%s]",
                   security_level_name(lib->required_level));
        printf(" (%d SUB", sub_count);
        if (func_count > 0)
            printf(", %d FUNCTION", func_count);
        if (def_fn_count > 0)
            printf(", %d DEF FN", def_fn_count);
        printf(")\n");
    }

    fclose(fp);

    // --- Pass 2: Compile blocks to PCode bytecode ---
    {
        BlockParseResult bpr;
        if (parser_block_compile_file(path, lib, &bpr) == 0) {
            printf("  Compiled: %d symbols, %d instructions"
                   " (%d lines)\n",
                   bpr.symbols_found, bpr.instructions,
                   bpr.lines_parsed);
        } else {
            printf("  Compiled with %d error(s).\n",
                   bpr.errors);
        }
    }

    return slot;
}

int lib_space_load_compiled(const char *path,
                            LibExtType ext_type, void *rt)
{
    int slot;

    (void)rt;

    // Delegate to lib_space_load_bpl which handles:
     //   - Duplicate checking
     //   - Slot allocation
     //   - bpl_load() (header validation, symbol/line deserialization)
    slot = lib_space_load_bpl(path);
    if (slot < 0) return -1;

    // Override the extension type from the caller's context
     // (the .BPL header may store a different type) 
    lib_slots[slot].ext_type = ext_type;

    return slot;
}

int lib_space_unload(const char *name)
{
    int j;
    for (int i = 0; i < lib_slot_count; i++) {
        if (lib_slots[i].loaded &&
            ls_str_iequal(lib_slots[i].name, name)) {
            // Free bytecode
            if (lib_slots[i].compiled) {
                pcode_free(&lib_slots[i].pcode);
            }
            // Free source line buffer
            if (lib_slots[i].src_lines) {
                free(lib_slots[i].src_lines);
                lib_slots[i].src_lines = NULL;
            }
            // Free string vars
            for (int j = 0; j < 26; j++) {
                if (lib_slots[i].str_vars[j]) {
                    free(lib_slots[i].str_vars[j]);
                    lib_slots[i].str_vars[j] = NULL;
                }
            }
            // Clear slot
            printf("Unloaded: %s\n", lib_slots[i].name);
            memset(&lib_slots[i], 0, sizeof(LoadedLibrary));
            return 0;
        }
    }
    printf("Library '%s' not found.\n", name);
    return -1;
}

LoadedLibrary *lib_space_find(const char *name)
{
    for (int i = 0; i < lib_slot_count; i++) {
        if (lib_slots[i].loaded &&
            ls_str_iequal(lib_slots[i].name, name))
            return &lib_slots[i];
    }
    return NULL;
}

LibSymbol *lib_space_find_symbol(const char *name,
                                 LoadedLibrary **out_lib)
{
    int j;
    for (int i = 0; i < lib_slot_count; i++) {
        if (!lib_slots[i].loaded) continue;
        for (int j = 0; j < lib_slots[i].symbol_count; j++) {
            if (lib_slots[i].symbols[j].active &&
                ls_str_iequal(lib_slots[i].symbols[j].name,
                              name)) {
                if (out_lib) *out_lib = &lib_slots[i];
                return &lib_slots[i].symbols[j];
            }
        }
    }
    if (out_lib) *out_lib = NULL;
    return NULL;
}

int lib_space_load_bpl(const char *path)
{
    int slot;
    LoadedLibrary *lib;

    // Check if already loaded
    {
        const char *base = path;
        const char *p = path;
        char name[MAX_LIB_NAME];
        while (*p) {
            if (*p == '/' || *p == '\\')
                base = p + 1;
            p++;
        }
        strncpy(name, base, MAX_LIB_NAME - 1);
        name[MAX_LIB_NAME - 1] = '\0';
        // Remove extension
        p = strrchr(name, '.');
        if (p) ((char *)name)[(int)(p - name)] = '\0';
        // Uppercase
        for (int i = 0; name[i]; i++)
            name[i] = (char)toupper((unsigned char)name[i]);
        // Check for duplicate
        for (int i = 0; i < lib_slot_count; i++) {
            if (lib_slots[i].loaded &&
                ls_str_iequal(lib_slots[i].name, name)) {
                printf("Library '%s' already loaded.\n",
                       name);
                return -1;
            }
        }
    }

    // Find a free slot
    slot = -1;
    {
        for (int i = 0; i < lib_slot_count; i++) {
            if (!lib_slots[i].loaded) {
                slot = i;
                break;
            }
        }
    }
    if (slot < 0) {
        if (lib_slot_count >= MAX_LOADED_LIBS) {
            printf("Library table full.\n");
            return -1;
        }
        slot = lib_slot_count++;
    }

    lib = &lib_slots[slot];
    memset(lib, 0, sizeof(LoadedLibrary));

    if (bpl_load(path, lib) != 0) {
        return -1;
    }

    return slot;
}

int lib_space_invoke(LoadedLibrary *lib, LibSymbol *sym,
                     BValue *args, int argc,
                     BValue *result, void *rt_ptr)
{
    RuntimeState *rt = (RuntimeState *)rt_ptr;
    int start_idx, li, end_found;
    int saved_current, saved_next, saved_running;
    int saved_in_sub;

    // Saved caller variables
    BValue saved_vars[MAX_VARIABLES];
    BValue saved_strvars[MAX_STRING_VARS];
    if (!lib || !sym) return -1;

    if (!lib->compiled || !lib->src_lines) {
        printf("Library '%s' not compiled.\n", lib->name);
        return -1;
    }

    if (sym->entry_offset < 0 ||
        sym->entry_offset >= lib->src_line_count) {
        printf("Symbol '%s' invalid entry point.\n",
               sym->name);
        return -1;
    }

    // Save caller's interpreter state
    saved_current = rt->current_index;
    saved_next = rt->next_index;
    saved_running = rt->running;
    saved_in_sub = rt->in_sub_index;

    // Save caller's variables
    int i;
    for (i = 0; i < MAX_VARIABLES; i++)
        saved_vars[i] = rt->variables[i];

    for (i = 0; i < MAX_STRING_VARS; i++)
        saved_strvars[i] = rt->string_vars[i];

    // Initialize library's variable space

    for (i = 0; i < 26; i++) {
        rt->variables[i] = bval_float(lib->num_vars[i]);
    }
    for (int i = 0; i < 26; i++) {
        rt->string_vars[i] = bval_float(0);
        if (lib->str_vars[i]) {
            rt->string_vars[i] = bval_string(
                lib->str_vars[i],
                (int)strlen(lib->str_vars[i]));
        }
    }

    // Push scope for LOCAL support a" MUST be before param binding
     // because SCOPE_FULL clears variables 
    {
        int smode = 1; // SCOPE_FULL
        scope_stack_push(&rt->scope_stack, rt,
                         smode, -1, -1);
    }

    // Bind parameters by parsing the declaration line to get
     // the actual parameter names (e.g., X, Y from FUNCTION F(X,Y)).
     // Single-letter params map to A-Z vars, multi-letter to named.
     // This MUST happen AFTER scope push to avoid being zeroed. 
    if (argc > 0 && args && sym->entry_offset < lib->src_line_count) {
        const char *decl = lib->src_lines[sym->entry_offset].text;
        const char *p = decl;
        int pi = 0;
        // Skip to opening paren
        while (*p && *p != '(') p++;
        if (*p == '(') {
            p++; // skip (
            while (*p && *p != ')' && pi < argc) {
                char pname[64];
                int plen = 0;
                // Skip whitespace and commas
                while (*p == ' ' || *p == ',' || *p == '\t') p++;
                if (*p == ')' || !*p) break;
                // Read parameter name
                while (*p && *p != ',' && *p != ')' &&
                       *p != ' ' && *p != '\t' &&
                       plen < 63) {
                    pname[plen++] = *p++;
                }
                pname[plen] = '\0';
                if (plen == 1 && pname[0] >= 'A' &&
                    pname[0] <= 'Z') {
                    rt->variables[pname[0] - 'A'] = args[pi];
                } else if (plen == 1 && pname[0] >= 'a' &&
                           pname[0] <= 'z') {
                    rt->variables[pname[0] - 'a'] = args[pi];
                } else if (plen > 0) {
                    runtime_set_named_var_bval(rt, pname,
                        plen, args[pi]);
                }
                pi++;
            }
        }
    }

    // Execute source lines from entry_offset+1
     // (skip the SUB/FUNCTION declaration line itself)
     // until END SUB / END FUNCTION 
    start_idx = sym->entry_offset + 1;
    end_found = 0;

    for (li = start_idx;
         li < lib->src_line_count && !end_found; li++) {
        const char *text = lib->src_lines[li].text;
        int vline = lib->src_lines[li].vline;
        const char *p = text;
        Lexer lex;

        // Skip whitespace
        while (*p == ' ' || *p == '\t') p++;

        // Check for END SUB / END FUNCTION
        if (toupper((unsigned char)p[0]) == 'E' &&
            toupper((unsigned char)p[1]) == 'N' &&
            toupper((unsigned char)p[2]) == 'D') {
            const char *ep = p + 3;
            while (*ep == ' ') ep++;
            if ((sym->type == LIB_SYM_SUB &&
                 toupper((unsigned char)ep[0]) == 'S' &&
                 toupper((unsigned char)ep[1]) == 'U' &&
                 toupper((unsigned char)ep[2]) == 'B') ||
                (sym->type == LIB_SYM_FUNCTION &&
                 toupper((unsigned char)ep[0]) == 'F' &&
                 toupper((unsigned char)ep[1]) == 'U' &&
                 toupper((unsigned char)ep[2]) == 'N' &&
                 toupper((unsigned char)ep[3]) == 'C')) {
                end_found = 1;
                break;
            }
        }

        // Skip REM lines
        if (toupper((unsigned char)p[0]) == 'R' &&
            toupper((unsigned char)p[1]) == 'E' &&
            toupper((unsigned char)p[2]) == 'M') {
            continue;
        }

        // Execute through the interpreter a" suppress errors
         // for library lines that use unsupported flow control
         // (FOR/NEXT, IF/ELSE blocks). Simple assignments, PRINT,
         // LOCAL, DIM all work. Complex flow control needs the
         // full interpreter pipeline and will silently fail. 
        {
            extern void error_set_suppress(int on);
            error_set_suppress(1);
            lexer_init(&lex, text);
            // Skip line number if present
            if (lex.current.type == TOK_NUMBER)
                lexer_next(&lex);

            error_clear();
            parser_execute_line(&lex, rt, vline);

            if (error_occurred()) {
                // Clear and continue a" don't stop on errors
                 // from flow control constructs 
                error_clear();
            }
            error_set_suppress(0);
        }
    }

    // Capture return value BEFORE scope pop a" the pop restores
     // named_count, which erases named vars set during execution 
    if (result) {
        if (sym->type == LIB_SYM_FUNCTION ||
            sym->type == LIB_SYM_DEF_FN) {
            // First check fn_return_value (set if in_sub_index
             // was configured correctly). If zero, fall back
             // to looking up the function name as a named var
             // (library code uses FUNCNAME = expr pattern). 
            *result = rt->fn_return_value;
            if (bval_to_float(result) == 0.0 &&
                !bval_is_string(result)) {
                BValue nv;
                nv = runtime_get_named_var_bval(
                    rt, sym->name, (int)strlen(sym->name));
                if (bval_to_float(&nv) != 0.0 ||
                    bval_is_string(&nv)) {
                    *result = nv;
                }
            }
        } else {
            *result = bval_int(0);
        }
    }

    // Pop scope (restores LOCALs and named var count)
    scope_stack_pop(&rt->scope_stack, rt);

    // Save library's variables back

    for (i = 0; i < 26; i++)
        lib->num_vars[i] = bval_to_float(&rt->variables[i]);

    // Restore caller's variables

    for (i = 0; i < MAX_VARIABLES; i++)
        rt->variables[i] = saved_vars[i];

    for (i = 0; i < MAX_STRING_VARS; i++)
        rt->string_vars[i] = saved_strvars[i];

    // Restore interpreter state
    rt->current_index = saved_current;
    rt->next_index = saved_next;
    rt->running = saved_running;
    rt->in_sub_index = saved_in_sub;

    return 0;
}

int lib_space_try_call(const char *name, int name_len,
                       BValue *args, int argc,
                       BValue *result, void *rt)
{
    char ubuf[MAX_LIB_NAME];
    LoadedLibrary *llib = NULL;
    LibSymbol *lsym = NULL;

    // Uppercase the name for lookup
    int i;
    for (i = 0; i < name_len && i < MAX_LIB_NAME - 1; i++) {
        ubuf[i] = (char)toupper((unsigned char)name[i]);
    }
    ubuf[i] = '\0';

    lsym = lib_space_find_symbol(ubuf, &llib);
    if (!lsym || !llib) return 0; // not found

    lib_space_invoke(llib, lsym, args, argc, result, rt);
    return 1; // found and called
}

void *lib_space_find_by_name(const char *name)
{
    for (int i = 0; i < lib_slot_count; i++) {
        if (lib_slots[i].loaded &&
            ls_str_iequal(lib_slots[i].name, name)) {
            return (void *)&lib_slots[i];
        }
    }
    return NULL;
}

int lib_space_try_call_func(const char *name, int name_len,
                             void *lex_ptr, void *rt_ptr,
                             int line_num, void *out_result)
{
    char ubuf[MAX_LIB_NAME];
    int argc;
    LoadedLibrary *llib = NULL;
    LibSymbol *lsym = NULL;
    BValue args[8];
    BValue result;
    Lexer *lex = (Lexer *)lex_ptr;
    extern BValue parse_expression_bval(Lexer *,
        RuntimeState *, int);

    // Uppercase the name for lookup
    int i;
    for (i = 0; i < name_len && i < MAX_LIB_NAME - 1; i++) {
        ubuf[i] = (char)toupper((unsigned char)name[i]);
    }
    ubuf[i] = '\0';

    lsym = lib_space_find_symbol(ubuf, &llib);
    if (!lsym || !llib) return 0; // not found
    if (lsym->type != LIB_SYM_FUNCTION) return 0;

    // Parse arguments from lexer: (arg1, arg2, ...)
    argc = 0;
    lexer_next(lex); // consume (
    if (lex->current.type != TOK_RPAREN) {
        args[argc] = parse_expression_bval(lex,
            (RuntimeState *)rt_ptr, line_num);
        if (error_occurred()) return 0;
        argc++;
        while (argc < 8 && lex->current.type == TOK_COMMA) {
            lexer_next(lex);
            args[argc] = parse_expression_bval(lex,
                (RuntimeState *)rt_ptr, line_num);
            if (error_occurred()) return 0;
            argc++;
        }
    }
    if (lex->current.type == TOK_RPAREN)
        lexer_next(lex); // consume )

    result = bval_int(0);
    lib_space_invoke(llib, lsym, args, argc, &result, rt_ptr);

    if (out_result) {
        *(BValue *)out_result = result;
    }
    return 1; // found and called
}

void lib_space_list(void)
{
    int j, found = 0;
    const char *type_names[] = {
        "LIB", "FN", "FT", "MOD", "PLG"
    };
    printf("--- Library Program Space ---\n");
    for (int i = 0; i < lib_slot_count; i++) {
        if (lib_slots[i].loaded) {
            printf("  [%s] %s",
                type_names[(int)lib_slots[i].ext_type],
                lib_slots[i].name);
            if (lib_slots[i].version[0])
                printf(" v%s", lib_slots[i].version);
            if (lib_slots[i].compiled)
                printf(" (compiled)");
            else
                printf(" (source)");
            printf(" - %d symbols\n",
                   lib_slots[i].symbol_count);
            // List symbols
            for (int j = 0; j < lib_slots[i].symbol_count; j++) {
                if (lib_slots[i].symbols[j].active) {
                    const char *stype = "???";
                    if (lib_slots[i].symbols[j].type ==
                        LIB_SYM_SUB) stype = "SUB";
                    else if (lib_slots[i].symbols[j].type ==
                        LIB_SYM_FUNCTION) stype = "FUNC";
                    else if (lib_slots[i].symbols[j].type ==
                        LIB_SYM_DEF_FN) stype = "DEF FN";
                    printf("    %s %s\n", stype,
                           lib_slots[i].symbols[j].name);
                }
            }
            found++;
        }
    }
    if (!found) printf("  (none)\n");
}

int lib_space_count(void)
{
    int n = 0;
    for (int i = 0; i < lib_slot_count; i++) {
        if (lib_slots[i].loaded) n++;
    }
    return n;
}

BValue *lib_space_get_var(LoadedLibrary *lib, const char *name)
{
    int i;
    if (!lib || !name) return NULL;

    // Check single-letter A-Z
    if (name[1] == '\0' || (name[1] == '$' && name[2] == '\0')) {
        int idx = toupper((unsigned char)name[0]) - 'A';
        if (idx >= 0 && idx < 26) {
            // Return pointer to the numeric value.
             // String vars handled separately. 
            // TODO: Return proper Value union pointer
            return NULL;
        }
    }

    // Search named variables
    for (i = 0; i < lib->named_var_count; i++) {
        if (lib->named_vars[i].active &&
            ls_str_iequal(lib->named_vars[i].name, name)) {
            return &lib->named_vars[i].val;
        }
    }
    return NULL;
}

int lib_space_set_var(LoadedLibrary *lib, const char *name,
                      BValue val)
{
    if (!lib || !name) return -1;
    int i;

    // Search existing named vars
    for (i = 0; i < lib->named_var_count; i++) {
        if (lib->named_vars[i].active &&
            ls_str_iequal(lib->named_vars[i].name, name)) {
            lib->named_vars[i].val = val;
            return 0;
        }
    }

    // Create new named var
    if (lib->named_var_count >= MAX_LIB_NAMED_VARS) {
        printf("Library '%s' named var limit reached.\n",
               lib->name);
        return -1;
    }
    i = lib->named_var_count;
    strncpy(lib->named_vars[i].name, name,
            MAX_LIB_NAME - 1);
    lib->named_vars[i].name[MAX_LIB_NAME - 1] = '\0';
    // Uppercase
    {
        char *p = lib->named_vars[i].name;
        while (*p) {
            *p = (char)toupper((unsigned char)*p);
            p++;
        }
    }
    lib->named_vars[i].val = val;
    lib->named_vars[i].is_string =
        (name[strlen(name) - 1] == '$') ? 1 : 0;
    lib->named_vars[i].active = 1;
    lib->named_var_count++;
    return 0;
}

 // lib_space_invoke_by_name - Convenience wrapper.
 //
 // Finds a symbol matching sym_name within the given library
 // and invokes it with zero arguments. Used by the spec
 // system to dispatch custom statements into their companion
 // .lib files.
 //
 // Returns 0 on success, -1 if symbol not found or error.
int lib_space_invoke_by_name(void *lib_ptr,
    const char *sym_name, void *rt)
{
    LoadedLibrary *lib = (LoadedLibrary *)lib_ptr;
    if (!lib || !sym_name) return -1;

    // Search for matching symbol in this library
    for (int j = 0; j < lib->symbol_count; j++) {
        if (lib->symbols[j].active &&
            ls_str_iequal(lib->symbols[j].name,
                          sym_name)) {
            BValue result;
            memset(&result, 0, sizeof(result));
            return lib_space_invoke(lib,
                &lib->symbols[j],
                NULL, 0, &result, rt);
        }
    }

    printf("Symbol '%s' not found in library "
           "'%s'.\n", sym_name, lib->name);
    return -1;
}
