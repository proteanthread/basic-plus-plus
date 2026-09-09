// FILENAME: find.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (file.h, file.c, language_descriptor.h)
// NEEDS: libcore (string.h)
// NEEDS: libengine (eval.h, eval.c, find.h, string.c)
// NEEDS: libkernel (errors.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the FIND statement in BASIC++.
//
// ---- Includes ----

#include "statements/filesystem/binary_ops/find.h"
#include "eval/eval.h"
#include "runtime/file.h"
#include "runtime/language_descriptor.h"
#include "types/errors.h"
#include "platform/platform.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "statements/variables/data/pick_locate.h"

static const LangDesc g_find_desc = {
    .name = "FIND",
    .category = "Filesystem I/O",
    .syntax = "FIND [#]channel [, RECORD record_number]",
    .description = "DEC RSTS/E RMS-11 statement to position the file pointer at a specific record without data transfer.",
    .error_summary = "Error 2: Syntax Error, Error 52: Bad File Number, Error 63: Bad Record Number",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

void stmt_find_register(void) {
    lang_desc_register(&g_find_desc);
}

BppError stmt_find_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    BppToken peek = lex_peek(lex);
    bool had_hash = false;
    if (peek.type == TOK_HASH) {
        lex_next(lex);
        had_hash = true;
    }

    if (!had_hash) {
        const char *in_pos = NULL;
        if (pick_locate_has_in_clause(lex, &in_pos)) {
            size_t tlen = (size_t)(in_pos - lex_get_pos(lex));
            char *tbuf = (char *)mem_scratch_alloc(vm_get_mem(vm), tlen + 1);
            if (tbuf) {
                runtime_memcpy(tbuf, lex_get_pos(lex), tlen);
                tbuf[tlen] = '\0';
                LexerContext *sub = lex_init(vm_get_mem(vm), tbuf);
                BValue target_val = eval_expression(vm, sub, &err);
                lex_shutdown(sub);
                if (err.code != 0) return err;
                lex_set_pos(lex, in_pos);
                return stmt_pick_locate_execute(vm, lex, target_val);
            }
        }
    }

    BValue ch_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (!had_hash) {
        BppToken after = lex_peek(lex);
        if ((after.type == TOK_KEYWORD && after.as.keyword == KW_IN) ||
            (after.type == TOK_IDENT && after.length == 2 && runtime_strncasecmp(after.start, "IN", 2) == 0)) {
            return stmt_pick_locate_execute(vm, lex, ch_val);
        }
    }
    if (ch_val.type != VAL_NUMBER && ch_val.type != VAL_INTEGER) {
        if (ch_val.type == VAL_STRING && ch_val.as.string) str_release(vm_get_str(vm), ch_val.as.string);
        err.code = ERR_TYPE_MISMATCH;
        return err;
    }
    int channel = (int)ch_val.as.number;

    FileContext *fc = vm_get_file(vm);
    if (!file_is_open(fc, channel)) {
        err.code = 52;
        err.message = "Bad File Number";
        return err;
    }

    long record_num = -1;
    BppToken comma = lex_peek(lex);
    if (comma.type == TOK_COMMA) {
        lex_next(lex); // Consume ','

        BppToken rec_kw = lex_peek(lex);
        if ((rec_kw.type == TOK_KEYWORD && rec_kw.as.keyword == KW_RECORD) ||
            (rec_kw.type == TOK_IDENT && rec_kw.length == 6 && platform_strncasecmp(rec_kw.start, "RECORD", 6) == 0)) {
            lex_next(lex); // Consume RECORD
        }

        BValue rec_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (rec_val.type == VAL_NUMBER || rec_val.type == VAL_INTEGER) {
            record_num = (long)rec_val.as.number;
        } else {
            if (rec_val.type == VAL_STRING && rec_val.as.string) str_release(vm_get_str(vm), rec_val.as.string);
        }
    }

    int rec_len = file_get_record_len(fc, channel);
    if (rec_len <= 0) rec_len = 128;

    if (record_num > 0) {
        file_seek(fc, channel, (record_num - 1) * rec_len);
    }

    return err;
}
