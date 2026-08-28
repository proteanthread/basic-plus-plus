// FILENAME: assign.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (file.h, file.c, micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libcore (string.h, strings.h, strings.c, variables.h, variables.c)
// NEEDS: libengine (assign.h, eval.h, eval.c, string.c, vm.h)
// NEEDS: libkernel (errors.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the ASSIGN statement in BASIC++.
//
// ---- Includes ----

#include "statements/filesystem/binary_ops/assign.h"
#include "eval/eval.h"
#include "runtime/file.h"
#include "runtime/variables.h"
#include "runtime/strings.h"
#include "runtime/micro_lib_metadata.h"
#include "types/errors.h"
#include "vm/vm.h"
#include "platform/platform.h"
#include <string.h>
#include <stdio.h>

void stmt_assign_register(void) {
    static const MicroLibMetadata meta_assign = {
        .name = "ASSIGN",
        .category = "Filesystem I/O",
        .syntax = "ASSIGN @Path TO \"filespec\" | ASSIGN @Path TO * | ASSIGN [#]channel TO \"filespec\" | ASSIGN \"filespec\" TO [#]channel",
        .help_text = "Dynamically associates a disk file specification or I/O device with an I/O path descriptor or channel (HP-3000 / HP-9845).",
        .error_codes = "Error 2: Syntax Error, Error 52: Bad File Number, Error 53: File Not Found"
    };
    microlib_register(&meta_assign);

    static const MicroLibMetadata meta_advance = {
        .name = "ADVANCE",
        .category = "Filesystem I/O",
        .syntax = "ADVANCE [#]channel, record_count",
        .help_text = "Advances or rewinds relative record position within an open channel (HP TSB).",
        .error_codes = "Error 2: Syntax Error, Error 52: Bad File Number, Error 62: Input Past End"
    };
    microlib_register(&meta_advance);
}

static int find_free_channel(FileContext *fc) {
    for (int ch = 1; ch <= 16; ch++) {
        if (!file_is_open(fc, ch)) return ch;
    }
    return -1;
}

BppError stmt_assign_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (!vm || !lex) {
        err.code = 5; err.message = "Null context";
        return err;
    }

    FileContext *fc = vm_get_file(vm);
    VariableContext *vc = vm_get_var(vm);
    BppToken first_tok = lex_peek(lex);

    // Form 1: ASSIGN @Path TO ...
    if (first_tok.type == TOK_AT || (first_tok.type == TOK_IDENT && first_tok.start && first_tok.start[0] == '@')) {
        char path_var[64] = "@";
        if (first_tok.type == TOK_AT) {
            lex_next(lex); // Consume '@'
            BppToken id_tok = lex_next(lex);
            if (id_tok.type != TOK_IDENT && id_tok.type != TOK_KEYWORD) {
                err.code = ERR_SYNTAX;
                err.message = "Expected identifier after '@' in ASSIGN";
                return err;
            }
            size_t ilen = (id_tok.length < sizeof(path_var) - 2) ? id_tok.length : sizeof(path_var) - 2;
            memcpy(path_var + 1, id_tok.start, ilen);
            path_var[1 + ilen] = '\0';
        } else {
            lex_next(lex);
            size_t ilen = (first_tok.length < sizeof(path_var) - 1) ? first_tok.length : sizeof(path_var) - 1;
            memcpy(path_var, first_tok.start, ilen);
            path_var[ilen] = '\0';
        }

        BppToken to_tok = lex_next(lex);
        bool is_to = (to_tok.type == TOK_KEYWORD && to_tok.as.keyword == KW_TO) ||
                     (to_tok.type == TOK_IDENT && to_tok.length == 2 && platform_strncasecmp(to_tok.start, "TO", 2) == 0);
        if (!is_to) {
            err.code = ERR_SYNTAX;
            err.message = "Expected 'TO' in ASSIGN statement";
            return err;
        }

        // Check for ASSIGN @Path TO * (stream close)
        BppToken dest_tok = lex_peek(lex);
        if (dest_tok.type == TOK_MUL) {
            lex_next(lex); // Consume '*'
            BValue *cur_val = var_lookup(vc, path_var, false);
            if (cur_val && (cur_val->type == VAL_NUMBER || cur_val->type == VAL_INTEGER)) {
                int ch = (int)cur_val->as.number;
                if (ch > 0 && file_is_open(fc, ch)) {
                    file_close(fc, ch);
                }
            }
            var_assign(vc, path_var, (BValue){.type = VAL_INTEGER, .as.number = 0.0});
            return err;
        }

        // Evaluate destination string (filename or device)
        BValue fn_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (fn_val.type != VAL_STRING || !fn_val.as.string) {
            if (fn_val.type == VAL_STRING && fn_val.as.string) str_release(vm_get_str(vm), fn_val.as.string);
            err.code = ERR_TYPE_MISMATCH;
            return err;
        }

        // Close existing channel if path already open
        BValue *cur_val = var_lookup(vc, path_var, false);
        int channel = -1;
        if (cur_val && (cur_val->type == VAL_NUMBER || cur_val->type == VAL_INTEGER)) {
            int old_ch = (int)cur_val->as.number;
            if (old_ch > 0 && file_is_open(fc, old_ch)) {
                file_close(fc, old_ch);
            }
        }

        channel = find_free_channel(fc);
        if (channel < 0) channel = 1;

        err = file_open(fc, vm_get_vdev(vm), channel, str_data(fn_val.as.string),
                        FILE_MODE_RANDOM, FILE_ACCESS_READ_WRITE, FILE_LOCK_DEFAULT, 128);
        str_release(vm_get_str(vm), fn_val.as.string);
        if (err.code == 0) {
            var_assign(vc, path_var, (BValue){.type = VAL_INTEGER, .as.number = (double)channel});
        }
        return err;
    }

    // Form 2: ASSIGN [#]channel TO "filespec" | ASSIGN [#]channel TO *
    if (first_tok.type == TOK_HASH || first_tok.type == TOK_NUMBER) {
        if (first_tok.type == TOK_HASH) lex_next(lex);
        BValue ch_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (ch_val.type != VAL_NUMBER && ch_val.type != VAL_INTEGER) {
            if (ch_val.type == VAL_STRING && ch_val.as.string) str_release(vm_get_str(vm), ch_val.as.string);
            err.code = ERR_TYPE_MISMATCH;
            return err;
        }
        int channel = (int)ch_val.as.number;

        BppToken to_tok = lex_next(lex);
        bool is_to = (to_tok.type == TOK_KEYWORD && to_tok.as.keyword == KW_TO) ||
                     (to_tok.type == TOK_IDENT && to_tok.length == 2 && platform_strncasecmp(to_tok.start, "TO", 2) == 0);
        if (!is_to) {
            err.code = ERR_SYNTAX;
            err.message = "Expected 'TO' in ASSIGN statement";
            return err;
        }

        BppToken dest_tok = lex_peek(lex);
        if (dest_tok.type == TOK_MUL) {
            lex_next(lex); // Consume '*'
            if (channel > 0 && file_is_open(fc, channel)) {
                file_close(fc, channel);
            }
            return err;
        }

        BValue fn_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (fn_val.type != VAL_STRING || !fn_val.as.string) {
            if (fn_val.type == VAL_STRING && fn_val.as.string) str_release(vm_get_str(vm), fn_val.as.string);
            err.code = ERR_TYPE_MISMATCH;
            return err;
        }

        if (channel > 0 && file_is_open(fc, channel)) {
            file_close(fc, channel);
        }

        err = file_open(fc, vm_get_vdev(vm), channel, str_data(fn_val.as.string),
                        FILE_MODE_RANDOM, FILE_ACCESS_READ_WRITE, FILE_LOCK_DEFAULT, 128);
        str_release(vm_get_str(vm), fn_val.as.string);
        return err;
    }

    // Form 3: ASSIGN "filespec" TO [#]channel | ASSIGN "filespec" TO @Path
    BValue fn_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (fn_val.type != VAL_STRING || !fn_val.as.string) {
        if (fn_val.type == VAL_STRING && fn_val.as.string) str_release(vm_get_str(vm), fn_val.as.string);
        err.code = ERR_TYPE_MISMATCH;
        return err;
    }

    BppToken tok = lex_next(lex);
    bool is_to = (tok.type == TOK_KEYWORD && tok.as.keyword == KW_TO) ||
                 (tok.type == TOK_IDENT && tok.length == 2 && platform_strncasecmp(tok.start, "TO", 2) == 0);
    if (!is_to) {
        str_release(vm_get_str(vm), fn_val.as.string);
        err.code = ERR_SYNTAX;
        err.message = "Expected 'TO' in ASSIGN statement";
        return err;
    }

    BppToken hash = lex_peek(lex);
    if (hash.type == TOK_AT || (hash.type == TOK_IDENT && hash.start && hash.start[0] == '@')) {
        char path_var[64] = "@";
        if (hash.type == TOK_AT) {
            lex_next(lex);
            BppToken id_tok = lex_next(lex);
            size_t ilen = (id_tok.length < sizeof(path_var) - 2) ? id_tok.length : sizeof(path_var) - 2;
            memcpy(path_var + 1, id_tok.start, ilen);
            path_var[1 + ilen] = '\0';
        } else {
            lex_next(lex);
            size_t ilen = (hash.length < sizeof(path_var) - 1) ? hash.length : sizeof(path_var) - 1;
            memcpy(path_var, hash.start, ilen);
            path_var[ilen] = '\0';
        }
        int channel = find_free_channel(fc);
        if (channel < 0) channel = 1;
        err = file_open(fc, vm_get_vdev(vm), channel, str_data(fn_val.as.string),
                        FILE_MODE_RANDOM, FILE_ACCESS_READ_WRITE, FILE_LOCK_DEFAULT, 128);
        str_release(vm_get_str(vm), fn_val.as.string);
        if (err.code == 0) {
            var_assign(vc, path_var, (BValue){.type = VAL_INTEGER, .as.number = (double)channel});
        }
        return err;
    }

    if (hash.type == TOK_HASH) {
        lex_next(lex);
    }

    BValue ch_val = eval_expression(vm, lex, &err);
    if (err.code != 0) {
        str_release(vm_get_str(vm), fn_val.as.string);
        return err;
    }
    if (ch_val.type != VAL_NUMBER && ch_val.type != VAL_INTEGER) {
        if (ch_val.type == VAL_STRING && ch_val.as.string) str_release(vm_get_str(vm), ch_val.as.string);
        str_release(vm_get_str(vm), fn_val.as.string);
        err.code = ERR_TYPE_MISMATCH;
        return err;
    }

    int channel = (int)ch_val.as.number;
    err = file_open(fc, vm_get_vdev(vm), channel, str_data(fn_val.as.string),
                    FILE_MODE_RANDOM, FILE_ACCESS_READ_WRITE, FILE_LOCK_DEFAULT, 128);
    str_release(vm_get_str(vm), fn_val.as.string);
    return err;
}

BppError stmt_advance_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken hash = lex_peek(lex);
    if (hash.type == TOK_HASH) {
        lex_next(lex);
    }

    BValue ch_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (ch_val.type != VAL_NUMBER && ch_val.type != VAL_INTEGER) {
        if (ch_val.type == VAL_STRING && ch_val.as.string) str_release(vm_get_str(vm), ch_val.as.string);
        err.code = ERR_TYPE_MISMATCH;
        return err;
    }
    int channel = (int)ch_val.as.number;

    BppToken comma = lex_next(lex);
    if (comma.type != TOK_COMMA) {
        err.code = ERR_SYNTAX;
        err.message = "Expected ',' in ADVANCE statement";
        return err;
    }

    BValue count_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (count_val.type != VAL_NUMBER && count_val.type != VAL_INTEGER) {
        if (count_val.type == VAL_STRING && count_val.as.string) str_release(vm_get_str(vm), count_val.as.string);
        err.code = ERR_TYPE_MISMATCH;
        return err;
    }

    // Advance relative record pointer
    int records = (int)count_val.as.number;
    long cur_pos = file_loc(vm_get_file(vm), channel);
    file_seek(vm_get_file(vm), channel, cur_pos + (long)records * 128);
    return err;
}
