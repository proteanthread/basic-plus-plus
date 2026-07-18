/**
 * @file stmt_program.c
 * @brief Program management statement command handlers (LIST, RUN, NEW).
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements program management commands:
 *   - LIST: Iterates over all stored program lines and prints them to "CON:".
 *   - RUN: Clears variable storage and sets the VM instruction pointer to the first line.
 *   - NEW: Clears both variable storage and the program line store.
 * - Why it exists: Provides core environment control commands for the REPL.
 * - Why it works this way: It interfaces with the MemoryContext and VariableContext.
 *   RUN starts execution by pointing next_line to the minimum line number, which the VM loop
 *   picks up and runs line-by-line.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Listing formats, list ranges (e.g. LIST 10-50).
 * - What cannot be changed: Memory clear guarantees of the NEW command.
 * - What to expect: Running the NEW command completely clears program memory.
 * - What to do if something breaks: If LIST fails, check the mem_program_get_all list retrieval loop.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Lines are stored sorted. Formatting outputs cleanly via vdev_printf.
 * - Portability concerns: None. C17 compliant.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add LOAD/SAVE file I/O hooks or LIST line ranges.
 * - How to write external extensions: Plugins do not override core program management commands.
 */

#include "bpp_stmt.h"
#include "bpp_vdev.h"
#include "bpp_platform.h"
#include "bpp_eval.h"
#include <string.h>
#include <stdio.h>

static bool parse_line_number(const char *str, BppLineNumber *out_line, const char **out_text);

/* LIST handler */
BppError stmt_list_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)lex;

    VDevContext *vdev = vm_get_vdev(vm);
    MemoryContext *mem = vm_get_mem(vm);

    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &count);

    for (size_t i = 0; i < count; ++i) {
        vdev_printf(vdev, "%g %s\n", lines[i].line_number, lines[i].text);
    }

    return err;
}

/* RUN handler */
BppError stmt_run_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    MemoryContext *mem = vm_get_mem(vm);

    BppToken tok = lex_peek(lex);
    bool has_arg = (tok.type != TOK_EOL && tok.type != TOK_EOF && tok.type != TOK_DOUBLE_COLON && tok.start[0] != ':');
    if (has_arg) {
        BValue val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (val.type != VAL_STRING) {
            err.code = 13; err.message = "Type mismatch: RUN expects filename string";
            return err;
        }
        const char *filename = str_data(val.as.string);
        char path[256];
        strncpy(path, filename, sizeof(path) - 1);
        path[sizeof(path) - 1] = '\0';
        str_release(vm_get_str(vm), val.as.string);

        err = vm_load_program_file(vm, path);
        if (err.code != 0) return err;
    }

    /* Reset loop stacks, variables, and exceptions */
    vm_reset_for_run(vm);

    /* Build DATA table */
    vm_build_data_table(vm);

    /* Set VM execution to the first line */
    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &count);

    if (count > 0) {
        /* Jump to the first line in the store */
        vm_jump(vm, lines[0].line_number, NULL);
    } else {
        /* No lines to run: NOP */
        vm_jump(vm, 0.0, NULL);
    }

    return err;
}

/* NEW handler */
BppError stmt_new_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)lex;

    MemoryContext *mem = vm_get_mem(vm);
    VariableContext *var = vm_get_var(vm);

    /* Clear variables and program */
    var_clear_all(var);
    mem_program_clear(mem);

    return err;
}

#include <ctype.h>

/* OPTION handler */
BppError stmt_option_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_next(lex);
    if (tok.type != TOK_IDENT) {
        err.code = 2;
        err.message = "Expected OPTION parameter (EXPLICIT or BASE)";
        return err;
    }

    char opt_name[64];
    size_t clen = (tok.length < 63) ? tok.length : 63;
    memcpy(opt_name, tok.as.string, clen);
    opt_name[clen] = '\0';

    /* Normalize to upper case */
    for (size_t j = 0; opt_name[j]; ++j) {
        opt_name[j] = (char)toupper((unsigned char)opt_name[j]);
    }

    if (strcmp(opt_name, "EXPLICIT") == 0) {
        bool enable = true;
        BppToken next = lex_peek(lex);
        if (next.type == TOK_IDENT || next.type == TOK_KEYWORD) {
            char val_name[64];
            size_t vlen = (next.length < 63) ? next.length : 63;
            memcpy(val_name, next.start, vlen);
            val_name[vlen] = '\0';
            for (size_t j = 0; val_name[j]; ++j) {
                val_name[j] = (char)toupper((unsigned char)val_name[j]);
            }
            if (strcmp(val_name, "ON") == 0) {
                lex_next(lex); /* Consume ON */
                enable = true;
            } else if (strcmp(val_name, "OFF") == 0) {
                lex_next(lex); /* Consume OFF */
                enable = false;
            }
        }
        var_set_explicit(vm_get_var(vm), enable);
    } else if (strcmp(opt_name, "BASE") == 0) {
        BppToken val_tok = lex_next(lex);
        if (val_tok.type != TOK_NUMBER) {
            err.code = 2;
            err.message = "Expected 0 or 1 after OPTION BASE";
            return err;
        }
        int base = (int)val_tok.as.number;
        if (base != 0 && base != 1) {
            err.code = 5;
            err.message = "OPTION BASE must be 0 or 1";
            return err;
        }
        arr_set_option_base(vm_get_arr(vm), base);
    } else {
        err.code = 2;
        err.message = "Unknown OPTION parameter";
    }

    return err;
}

#include <stdlib.h>

static bool parse_line_number(const char *str, BppLineNumber *out_line, const char **out_text) {
    while (*str && isspace((unsigned char)*str)) {
        str++;
    }
    if (!isdigit((unsigned char)*str)) {
        return false;
    }
    char *endptr;
    double line = strtod(str, &endptr);
    *out_line = line;
    *out_text = endptr;
    return true;
}

BppError stmt_load_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_next(lex);
    if (tok.type != TOK_STRING) {
        err.code = 2;
        err.message = "Expected string literal filename in LOAD";
        return err;
    }

    char filename[256];
    size_t len = (tok.length < sizeof(filename) - 1) ? tok.length : sizeof(filename) - 1;
    memcpy(filename, tok.as.string, len);
    filename[len] = '\0';

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        err.code = 53; /* File not found */
        err.message = "File not found during LOAD";
        return err;
    }

    MemoryContext *mem = vm_get_mem(vm);
    VariableContext *var = vm_get_var(vm);

    /* LOAD clears program and variables */
    var_clear_all(var);
    mem_program_clear(mem);

    char line_buf[1024];
    while (fgets(line_buf, sizeof(line_buf), fp)) {
        size_t slen = strlen(line_buf);
        while (slen > 0 && isspace((unsigned char)line_buf[slen - 1])) {
            line_buf[slen - 1] = '\0';
            slen--;
        }

        char *ptr = line_buf;
        while (*ptr && isspace((unsigned char)*ptr)) {
            ptr++;
        }
        if (*ptr == '\0') continue;

        BppLineNumber line_num = 0.0;
        const char *stmt_text = NULL;
        if (parse_line_number(ptr, &line_num, &stmt_text)) {
            while (*stmt_text && isspace((unsigned char)*stmt_text)) {
                stmt_text++;
            }
            if (*stmt_text != '\0') {
                mem_program_insert(mem, line_num, stmt_text);
            }
        }
    }

    fclose(fp);
    return err;
}

BppError stmt_save_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_next(lex);
    if (tok.type != TOK_STRING) {
        err.code = 2;
        err.message = "Expected string literal filename in SAVE";
        return err;
    }

    char filename[256];
    size_t len = (tok.length < sizeof(filename) - 1) ? tok.length : sizeof(filename) - 1;
    memcpy(filename, tok.as.string, len);
    filename[len] = '\0';

    FILE *fp = fopen(filename, "w");
    if (!fp) {
        err.code = 61; /* Disk full / Write error */
        err.message = "Failed to open file for writing in SAVE";
        return err;
    }

    MemoryContext *mem = vm_get_mem(vm);
    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &count);

    for (size_t i = 0; i < count; ++i) {
        fprintf(fp, "%g %s\n", lines[i].line_number, lines[i].text);
    }

    fclose(fp);
    return err;
}

BppError stmt_merge_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_next(lex);
    if (tok.type != TOK_STRING) {
        err.code = 2;
        err.message = "Expected string literal filename in MERGE";
        return err;
    }

    char filename[256];
    size_t len = (tok.length < sizeof(filename) - 1) ? tok.length : sizeof(filename) - 1;
    memcpy(filename, tok.as.string, len);
    filename[len] = '\0';

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        err.code = 53;
        err.message = "File not found during MERGE";
        return err;
    }

    MemoryContext *mem = vm_get_mem(vm);

    char line_buf[1024];
    while (fgets(line_buf, sizeof(line_buf), fp)) {
        size_t slen = strlen(line_buf);
        while (slen > 0 && isspace((unsigned char)line_buf[slen - 1])) {
            line_buf[slen - 1] = '\0';
            slen--;
        }

        char *ptr = line_buf;
        while (*ptr && isspace((unsigned char)*ptr)) {
            ptr++;
        }
        if (*ptr == '\0') continue;

        BppLineNumber line_num = 0.0;
        const char *stmt_text = NULL;
        if (parse_line_number(ptr, &line_num, &stmt_text)) {
            while (*stmt_text && isspace((unsigned char)*stmt_text)) {
                stmt_text++;
            }
            if (*stmt_text != '\0') {
                mem_program_insert(mem, line_num, stmt_text);
            }
        }
    }

    fclose(fp);
    return err;
}

/**
 * @brief ENVIRON "VAR=VALUE"
 */
BppError stmt_environ_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BValue env_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (env_val.type != VAL_STRING) {
        if (env_val.type == VAL_STRING) str_release(vm_get_str(vm), env_val.as.string);
        err.code = 13; err.message = "Type mismatch (expected string for ENVIRON)";
        return err;
    }

    const char *env_str = str_data(env_val.as.string);
    const char *eq = strchr(env_str, '=');
    if (!eq) {
        str_release(vm_get_str(vm), env_val.as.string);
        err.code = 5; err.message = "Illegal function call (expected VAR=VALUE)";
        return err;
    }

    size_t name_len = eq - env_str;
    char name[256];
    if (name_len >= sizeof(name)) name_len = sizeof(name) - 1;
    strncpy(name, env_str, name_len);
    name[name_len] = '\0';

    const char *val = eq + 1;

    platform_setenv(name, val);

    str_release(vm_get_str(vm), env_val.as.string);
    return err;
}
