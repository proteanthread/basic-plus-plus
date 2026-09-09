// FILENAME: stmt_pipe.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h), libengine (eval.h, stmt.h), libkernel (vdev.h)
// Provides runtime implementation for PIPE and STREAMPIPE statements in BASIC++.
//
// ---- Includes ----

#include "statements/filesystem/file_ops/stmt_pipe.h"
#include "eval/eval.h"
#include "runtime/file.h"
#include "runtime/language_descriptor.h"
#include "core/dialect.h"
#include "lexer/lexer.h"
#include "types/errors.h"

#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/format/snprintf.h"

static const LangDesc g_pipe_desc = {
    .name = "PIPE",
    .category = "Filesystem & I/O",
    .syntax = "PIPE #src_ch TO #dst_ch [BUFFER size] | STREAMPIPE #src, #dst",
    .description = "Streams data continuously from source file/device channel to destination channel.",
    .error_summary = "Error 2: Syntax Error, Error 52: Bad File Number, Error 7: Out of Memory",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

BppError stmt_pipe_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    if (!vm || !lex) {
        err.code = 5; err.message = "Null VM or lexer context";
        return err;
    }

    FileContext *fc = vm_get_file(vm);
    if (!fc) {
        err.code = 52; err.message = "Bad File Number";
        return err;
    }

    // 1. Parse source channel
    if (lex_peek(lex).type == TOK_HASH) {
        lex_next(lex);
    }
    BValue src_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    int src_ch = (int)src_val.as.number;

    if (!file_is_open(fc, src_ch)) {
        err.code = 52; err.message = "Source channel not open";
        return err;
    }

    // 2. Consume separator (TO or COMMA)
    BppToken sep = lex_peek(lex);
    if (sep.type == TOK_KEYWORD && sep.as.keyword == KW_TO) {
        lex_next(lex);
    } else if (sep.type == TOK_IDENT && sep.length == 2 && runtime_strncasecmp(sep.start, "TO", 2) == 0) {
        lex_next(lex);
    } else if (sep.type == TOK_COMMA) {
        lex_next(lex);
    } else {
        err.code = 2; err.message = "Expected TO or comma between pipe channels";
        return err;
    }

    // 3. Parse destination channel
    if (lex_peek(lex).type == TOK_HASH) {
        lex_next(lex);
    }
    BValue dst_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    int dst_ch = (int)dst_val.as.number;

    if (!file_is_open(fc, dst_ch)) {
        err.code = 52; err.message = "Destination channel not open";
        return err;
    }

    // 4. Check optional BUFFER size
    size_t chunk_size = 4096;
    BppToken buf_tok = lex_peek(lex);
    if ((buf_tok.type == TOK_IDENT && buf_tok.length == 6 && runtime_strncasecmp(buf_tok.start, "BUFFER", 6) == 0) ||
        (buf_tok.type == TOK_IDENT && buf_tok.length == 3 && runtime_strncasecmp(buf_tok.start, "BUF", 3) == 0)) {
        lex_next(lex);
        BValue sz_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (sz_val.as.number > 0 && sz_val.as.number <= 65536) {
            chunk_size = (size_t)sz_val.as.number;
        }
    }

    char *buf = (char *)runtime_malloc(chunk_size + 1);
    if (!buf) {
        err.code = 7; err.message = "Out of Memory";
        return err;
    }

    // 5. Transfer loop
    while (!file_eof(fc, src_ch)) {
        int bytes_read = file_read(fc, src_ch, buf, (int)chunk_size);
        if (bytes_read > 0) {
            file_write_raw(fc, dst_ch, buf, (size_t)bytes_read);
        } else {
            // Line-based fallback for virtual devices (BUS:, IPC:, etc.)
            if (file_gets(fc, src_ch, buf, (size_t)chunk_size)) {
                size_t l = runtime_strlen(buf);
                if (l > 0) {
                    file_puts(fc, dst_ch, buf);
                }
            } else {
                break; // No more data available
            }
        }
    }

    runtime_free(buf);
    return err;
}

void stmt_pipe_register(void) {
    lang_desc_register(&g_pipe_desc);
}
