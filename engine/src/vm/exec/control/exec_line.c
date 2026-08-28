// FILENAME: exec_line.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libengine (exec_control_internal.h)
// Implements bytecode virtual machine execution and state for exec_line.
//
// ---- Includes ----

#include "vm/exec_control_internal.h"

//
// ---- Namespace Resolution ----

static void get_namespace_at_line(VMContext *vm, BppLineNumber target_line, char *out_ns, size_t max_len) {
    if (!vm || !out_ns || max_len == 0) return;
    out_ns[0] = '\0';
    if (!mem_program_has_namespaces(vm_get_mem(vm))) {
        return;
    }
    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(vm_get_mem(vm), &count);
    char current_ns[64] = "";

    for (size_t i = 0; i < count; i++) {
        if (lines[i].line_number > target_line) {
            break;
        }
        if (strstr(lines[i].text, "NAMESPACE") || strstr(lines[i].text, "namespace") || strstr(lines[i].text, "Namespace")) {
            LexerContext *scan_lex = lex_init(vm_get_mem(vm), lines[i].text);
            if (scan_lex) {
                BppToken tok = lex_next(scan_lex);
                if (tok.type == TOK_NAMESPACE_DECL) {
                    int len = (int)(tok.length < sizeof(current_ns) - 1 ? tok.length : sizeof(current_ns) - 1);
                    memcpy(current_ns, tok.as.string, len);
                    current_ns[len] = '\0';
                    if (strcasecmp(current_ns, "DEFAULT") == 0) {
                        current_ns[0] = '\0';
                    }
                }
                lex_shutdown(scan_lex);
            }
        }
    }
    size_t copy_len = strlen(current_ns);
    if (copy_len >= max_len) copy_len = max_len - 1;
    memcpy(out_ns, current_ns, copy_len);
    out_ns[copy_len] = '\0';
}

//
// ---- Line Execution Loop ----

BppError vm_execute_line(VMContext *vm, const char *source) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (!vm || !source) return err;

    size_t slen = strlen(source);
    char stack_buf[1024];
    char *source_copy = (slen < sizeof(stack_buf)) ? stack_buf : (char *)malloc(slen + 1);
    if (!source_copy) {
        err.code = 14;
        err.message = "Failed to allocate statement line buffer";
        return err;
    }
    memcpy(source_copy, source, slen + 1);

    const char *prev_orig = vm->active_line_original;
    const char *prev_copy = vm->active_line_copy;
    const char *prev_pos  = vm->current_pos;

    vm->active_line_original = source;
    vm->active_line_copy = source_copy;

    char ns[64];
    get_namespace_at_line(vm, vm->current_line, ns, sizeof(ns));
    var_set_namespace(vm->var, ns);

    const char *start_pos = source_copy;
    if (vm->current_pos && vm->current_pos >= source && vm->current_pos <= source + slen) {
        ptrdiff_t offset = vm->current_pos - source;
        if (offset >= 0 && (size_t)offset <= slen) {
            start_pos = source_copy + offset;
        }
    }
    vm->current_pos = NULL;
    vm->eval_depth = 0;

    LexerContext *lex = lex_init(vm->mem, start_pos);
    if (!lex) {
        vm->active_line_original = prev_orig;
        vm->active_line_copy = prev_copy;
        vm->current_pos = prev_pos;
        if (source_copy != stack_buf) free(source_copy);
        err.code = 14;
        err.message = "Failed to initialize statement parser";
        return err;
    }

    bool was_running = vm->running;
    BppToken tok = lex_peek(lex);
    while (tok.type != TOK_EOF && (was_running ? vm->running : true)) {
        ptrdiff_t offset = tok.start - source_copy;
        vm->current_pos = source + offset;

        err = execute_single_statement(vm, lex);
        if (err.code != 0) {
            if (file_txn_status(vm->file) == 2) {
                file_txn_rollback(vm->file);
            }
            ptrdiff_t next_offset = lex_get_pos(lex) - source_copy;
            vm->next_pos = source + next_offset;
            break;
        }

        if (vm->jump_active) {
            if (vm->next_line == vm->current_line) {
                ptrdiff_t jmp_offset = -1;
                if (vm->next_pos && vm->next_pos >= source && vm->next_pos <= source + slen) {
                    jmp_offset = vm->next_pos - source;
                } else if (vm->next_pos && vm->next_pos >= source_copy && vm->next_pos <= source_copy + slen) {
                    jmp_offset = vm->next_pos - source_copy;
                }
                if (jmp_offset >= 0 && (size_t)jmp_offset <= slen) {
                    lex_shutdown(lex);
                    lex = lex_init(vm->mem, source_copy + jmp_offset);
                    if (lex) {
                        vm->jump_active = false;
                        tok = lex_peek(lex);
                        continue;
                    }
                }
            }
            break;
        }

        tok = lex_peek(lex);
        ptrdiff_t next_offset = tok.start - source_copy;
        vm->next_pos = source + next_offset;

        if (tok.type == TOK_EOL || tok.type == TOK_BACKSLASH) {
            lex_next(lex);
            tok = lex_peek(lex);
        }
    }

    lex_shutdown(lex);
    vm->active_line_original = prev_orig;
    vm->active_line_copy = prev_copy;
    vm->current_pos = prev_pos;
    if (source_copy != stack_buf) free(source_copy);

    if (!was_running && vprinter_has_output()) {
        vprinter_flush_pdf(NULL);
    }

    return err;
}
