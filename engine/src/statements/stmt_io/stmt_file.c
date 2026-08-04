/**
 * @file stmt_file.c
 * @brief Statement handlers for File system and Record I/O.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements OPEN, CLOSE, GET, PUT, SEEK statements,
 *   as well as PRINT #, WRITE #, INPUT #, and LINE INPUT (#) statement logic.
 * - Why it exists: Provides file manipulation and structured database operations
 *   for BASIC++ programs.
 * - Why it works this way: It interfaces with the FileContext manager. Tokens are parsed
 *   ephemerally. Expression results are formatted and written or read directly.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Parsing rules for record buffers or delimiters.
 * - What cannot be changed: Obligation to check channel bounds and active statuses.
 * - What to expect: Files are flushed automatically on CLOSE.
 * - What to do if something breaks: If a file fails to write, verify channel mode (OUTPUT/APPEND).
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: File channels are positive integers (1 to 16).
 * - Portability concerns: None. C17 standard compliant.
 */

#include "stmt/stmt.h"
#include "runtime/file.h"
#include "eval/eval.h"
#include "runtime/num_format.h"
#include "device/vdev.h"
#include "security/security.h"
#include "runtime/vfs.h"
#include "runtime/vnet.h"
#include "vm/vm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void print_using_internal(VMContext *vm, LexerContext *lex, int channel);

/* Forward declare console gets helper */
static bool get_console_line(VMContext *vm, char *buf, size_t size) {
    VDevContext *vdev = vm_get_vdev(vm);
    VDev *con = vdev_get(vdev, "CON:");
    if (!con || !con->ops.gets) return false;
    return con->ops.gets(con, buf, (int)size) != NULL;
}



/**
 * @brief Helper to read a token/field from a comma-separated stream.
 */
static bool read_field(FileContext *ctx, int channel, char *buf, size_t size) {
    size_t len = 0;
    int c;
    bool in_quotes = false;

    /* Skip leading whitespace (except newlines/commas) */
    while ((c = file_getc(ctx, channel)) != -1) {
        if (c == '\n' || c == '\r' || c == ',') {
            file_ungetc(ctx, channel, c);
            break;
        }
        if (!isspace(c)) {
            file_ungetc(ctx, channel, c);
            break;
        }
    }

    while ((c = file_getc(ctx, channel)) != -1) {
        if (c == '"') {
            in_quotes = !in_quotes;
            continue;
        }
        if (!in_quotes) {
            if (c == ',' || c == '\n' || c == '\r') {
                /* Field ends. If comma, consume it. If newline, keep it for next reads. */
                if (c != ',') {
                    file_ungetc(ctx, channel, c);
                }
                break;
            }
        }
        if (len < size - 1) {
            buf[len++] = (char)c;
        }
    }
    buf[len] = '\0';
    return (len > 0 || c != -1);
}

/**
 * @brief OPEN "filename" FOR [INPUT|OUTPUT|APPEND|BINARY|RANDOM] AS #channel [LEN = record_len]
 */
BppError stmt_open_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    /* 1. File Name Expression */
    BValue name_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (name_val.type != VAL_STRING) {
        err.code = 13; err.message = "Type mismatch: expected string for filename";
        return err;
    }
    const char *filename = str_data(name_val.as.string);

    /* 2. FOR mode */
    BppToken tok = lex_next(lex);
    if (tok.type != TOK_KEYWORD || tok.as.keyword != KW_FOR) {
        err.code = 2; err.message = "Expected FOR keyword in OPEN statement";
        str_release(vm_get_str(vm), name_val.as.string);
        return err;
    }

    /* 3. Mode Identifier */
    tok = lex_next(lex);
    char mode_str[64] = {0};
    size_t mlen = (tok.length < sizeof(mode_str) - 1) ? tok.length : sizeof(mode_str) - 1;
    memcpy(mode_str, tok.start, mlen);
    for (size_t i = 0; i < mlen; i++) mode_str[i] = (char)toupper((unsigned char)mode_str[i]);

    BppFileMode mode = FILE_MODE_INPUT;
    if (strcmp(mode_str, "INPUT") == 0)      mode = FILE_MODE_INPUT;
    else if (strcmp(mode_str, "OUTPUT") == 0) mode = FILE_MODE_OUTPUT;
    else if (strcmp(mode_str, "APPEND") == 0) mode = FILE_MODE_APPEND;
    else if (strcmp(mode_str, "BINARY") == 0) mode = FILE_MODE_BINARY;
    else if (strcmp(mode_str, "RANDOM") == 0) mode = FILE_MODE_RANDOM;
    else {
        err.code = 2; err.message = "Bad file mode: expected INPUT, OUTPUT, APPEND, BINARY, or RANDOM";
        str_release(vm_get_str(vm), name_val.as.string);
        return err;
    }

    /* Security Gates */
    BppSecOperation op = SECOP_FILE_READ;
    if (mode == FILE_MODE_OUTPUT || mode == FILE_MODE_APPEND) {
        op = SECOP_FILE_WRITE;
    } else if (mode == FILE_MODE_BINARY || mode == FILE_MODE_RANDOM) {
        op = SECOP_FILE_BLOCK;
    }
    if (security_check(op, 0) != 0) {
        err.code = 70; err.message = "Permission denied";
        str_release(vm_get_str(vm), name_val.as.string);
        return err;
    }

    BppFileAccess access = FILE_ACCESS_DEFAULT;
    BppFileLockMode lock_mode = FILE_LOCK_DEFAULT;

    /* Parse optional ACCESS and SHARED/LOCK */
    while (true) {
        tok = lex_peek(lex);
        if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_AS) break;
        if (tok.type == TOK_EOF || tok.type == TOK_EOL) break;
        
        char temp[64] = {0};
        size_t tlen = (tok.length < sizeof(temp) - 1) ? tok.length : sizeof(temp) - 1;
        memcpy(temp, tok.start, tlen);
        for (size_t i = 0; i < tlen; i++) temp[i] = (char)toupper((unsigned char)temp[i]);

        if (strcmp(temp, "ACCESS") == 0) {
            lex_next(lex); /* Consume ACCESS */
            tok = lex_peek(lex);
            tlen = (tok.length < sizeof(temp) - 1) ? tok.length : sizeof(temp) - 1;
            memcpy(temp, tok.start, tlen);
            for (size_t i = 0; i < tlen; i++) temp[i] = (char)toupper((unsigned char)temp[i]);
            
            if (strcmp(temp, "READ") == 0) {
                lex_next(lex);
                tok = lex_peek(lex);
                tlen = (tok.length < sizeof(temp) - 1) ? tok.length : sizeof(temp) - 1;
                memcpy(temp, tok.start, tlen);
                for (size_t i = 0; i < tlen; i++) temp[i] = (char)toupper((unsigned char)temp[i]);
                if (strcmp(temp, "WRITE") == 0) {
                    lex_next(lex);
                    access = FILE_ACCESS_READ_WRITE;
                } else {
                    access = FILE_ACCESS_READ;
                }
            } else if (strcmp(temp, "WRITE") == 0) {
                lex_next(lex);
                access = FILE_ACCESS_WRITE;
            } else {
                err.code = 2; err.message = "Expected READ or WRITE after ACCESS";
                str_release(vm_get_str(vm), name_val.as.string);
                return err;
            }
        } else if (strcmp(temp, "SHARED") == 0) {
            lex_next(lex);
            lock_mode = FILE_LOCK_SHARED;
        } else if (strcmp(temp, "LOCK") == 0) {
            lex_next(lex); /* Consume LOCK */
            tok = lex_peek(lex);
            tlen = (tok.length < sizeof(temp) - 1) ? tok.length : sizeof(temp) - 1;
            memcpy(temp, tok.start, tlen);
            for (size_t i = 0; i < tlen; i++) temp[i] = (char)toupper((unsigned char)temp[i]);
            
            if (strcmp(temp, "READ") == 0) {
                lex_next(lex);
                tok = lex_peek(lex);
                tlen = (tok.length < sizeof(temp) - 1) ? tok.length : sizeof(temp) - 1;
                memcpy(temp, tok.start, tlen);
                for (size_t i = 0; i < tlen; i++) temp[i] = (char)toupper((unsigned char)temp[i]);
                if (strcmp(temp, "WRITE") == 0) {
                    lex_next(lex);
                    lock_mode = FILE_LOCK_READ_WRITE;
                } else {
                    lock_mode = FILE_LOCK_READ;
                }
            } else if (strcmp(temp, "WRITE") == 0) {
                lex_next(lex);
                lock_mode = FILE_LOCK_WRITE;
            } else {
                err.code = 2; err.message = "Expected READ or WRITE after LOCK";
                str_release(vm_get_str(vm), name_val.as.string);
                return err;
            }
        } else {
            err.code = 2; err.message = "Unexpected token before AS";
            str_release(vm_get_str(vm), name_val.as.string);
            return err;
        }
    }

    /* 4. AS keyword */
    tok = lex_next(lex);
    if (tok.type != TOK_KEYWORD || tok.as.keyword != KW_AS) {
        err.code = 2; err.message = "Expected AS keyword in OPEN statement";
        str_release(vm_get_str(vm), name_val.as.string);
        return err;
    }

    /* 5. '#' channel prefix (optional but recommended) */
    tok = lex_peek(lex);
    if (tok.type == TOK_HASH) {
        lex_next(lex); /* Consume '#' */
    }

    /* 6. Channel number expression */
    BValue ch_val = eval_expression(vm, lex, &err);
    if (err.code != 0) {
        str_release(vm_get_str(vm), name_val.as.string);
        return err;
    }
    if (ch_val.type == VAL_STRING && ch_val.as.string) {
        str_release(vm_get_str(vm), ch_val.as.string);
    }
    if (ch_val.type != VAL_NUMBER) {
        err.code = 13;
        err.message = "Type mismatch";
        str_release(vm_get_str(vm), name_val.as.string);
        return err;
    }
    int channel = (int)ch_val.as.number;

    /* 7. Optional LEN = record_len */
    int record_len = 0;
    tok = lex_peek(lex);
    if (tok.type == TOK_IDENT) {
        char temp[64] = {0};
        size_t tlen = (tok.length < sizeof(temp) - 1) ? tok.length : sizeof(temp) - 1;
        memcpy(temp, tok.start, tlen);
        for (size_t i = 0; i < tlen; i++) temp[i] = (char)toupper((unsigned char)temp[i]);

        if (strcmp(temp, "LEN") == 0) {
            lex_next(lex); /* Consume 'LEN' */
            tok = lex_next(lex);
            if (tok.type != TOK_EQ) {
                err.code = 2; err.message = "Expected '=' after LEN";
                str_release(vm_get_str(vm), name_val.as.string);
                return err;
            }
            BValue len_val = eval_expression(vm, lex, &err);
            if (err.code != 0) {
                str_release(vm_get_str(vm), name_val.as.string);
                return err;
            }
            if (len_val.type == VAL_STRING && len_val.as.string) {
                str_release(vm_get_str(vm), len_val.as.string);
            }
            if (len_val.type != VAL_NUMBER) {
                err.code = 13;
                err.message = "Type mismatch";
                str_release(vm_get_str(vm), name_val.as.string);
                return err;
            }
            record_len = (int)len_val.as.number;
        }
    }

    char resolved[512];
    vfs_resolve(vm_get_vfs(vm), filename, resolved, sizeof(resolved));

    if (strncmp(resolved, "TCP:", 4) == 0 || strncmp(resolved, "UDP:", 4) == 0) {
        char temp_res[520];
        snprintf(temp_res, sizeof(temp_res), "NET:%s", resolved);
        strncpy(resolved, temp_res, sizeof(resolved) - 1);
        resolved[sizeof(resolved) - 1] = '\0';
    }

#if BPP_SUPPORT_NET
    /* VNet Routing check */
    if (strncmp(resolved, "NET:TCP:", 8) == 0 || strncmp(resolved, "NET:UDP:", 8) == 0) {
        char protocol[16];
        char host[256];
        int port = 80;

        /* Parse protocol */
        if (strncmp(resolved + 4, "TCP:", 4) == 0) snprintf(protocol, sizeof(protocol), "TCP");
        else snprintf(protocol, sizeof(protocol), "UDP");

        /* Parse host and port */
        const char *host_start = resolved + 8;
        const char *colon = strchr(host_start, ':');
        if (colon) {
            size_t host_len = colon - host_start;
            if (host_len < sizeof(host)) {
                memcpy(host, host_start, host_len);
                host[host_len] = '\0';
                char *endptr;
                port = (int)strtol(colon + 1, &endptr, 10);
                if (endptr == colon + 1) {
                    port = 80;
                }
            }
        } else {
            strncpy(host, host_start, sizeof(host) - 1);
            host[sizeof(host) - 1] = '\0';
        }

        /* Register NET: device dynamically */
        VDev ndev = vnet_create_vdev(vm_get_vnet(vm), "NET:", protocol, host, port);
        vdev_register(vm_get_vdev(vm), ndev);

        /* Set filename to "NET:" so file_open matches the device */
        snprintf(resolved, sizeof(resolved), "NET:");
    }
#endif

    err = file_open(vm_get_file(vm), vm_get_vdev(vm), channel, resolved, mode, access, lock_mode, record_len);
    str_release(vm_get_str(vm), name_val.as.string);
    return err;
}

/**
 * @brief CLOSE [#channel1, #channel2, ...]
 */
BppError stmt_close_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_EOL || tok.type == TOK_EOF) {
        /* Close all files */
        file_close_all(vm_get_file(vm));
        return err;
    }

    while (true) {
        tok = lex_peek(lex);
        if (tok.type == TOK_HASH) {
            lex_next(lex); /* Consume '#' */
        }
        BValue ch_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (ch_val.type == VAL_STRING && ch_val.as.string) {
            str_release(vm_get_str(vm), ch_val.as.string);
        }
        if (ch_val.type != VAL_NUMBER) {
            err.code = 13;
            err.message = "Type mismatch";
            return err;
        }
        file_close(vm_get_file(vm), (int)ch_val.as.number);

        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA) {
            lex_next(lex); /* Consume ',' */
        } else {
            break;
        }
    }

    return err;
}

/**
 * @brief RESET
 * Closes all open file channels (1 through 255) and flushes active buffers.
 */
BppError stmt_reset_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    file_close_all(vm_get_file(vm));
    return err;
}

/**
 * @brief GET #channel, [record_number], variable
 */
BppError stmt_kbd_get_handler(VMContext *vm, LexerContext *lex);
BppError stmt_kbd_get_using_handler(VMContext *vm, LexerContext *lex);

BppError stmt_get_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_USING) {
        return stmt_kbd_get_using_handler(vm, lex);
    }
    if (tok.type == TOK_IDENT) {
        return stmt_kbd_get_handler(vm, lex);
    }

    if (tok.type == TOK_HASH) {
        lex_next(lex);
    }

    BValue ch_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (ch_val.type == VAL_STRING && ch_val.as.string) {
        str_release(vm_get_str(vm), ch_val.as.string);
    }
    if (ch_val.type != VAL_NUMBER) {
        err.code = 13;
        err.message = "Type mismatch";
        return err;
    }
    int channel = (int)ch_val.as.number;

    tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex);
    }

    long record = -1;
    tok = lex_peek(lex);
    if (tok.type != TOK_EOF && tok.type != TOK_EOL && tok.type != TOK_COMMA) {
        if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_ELSE) {
            /* Stop parsing */
        } else {
            BValue rec_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (rec_val.type == VAL_STRING && rec_val.as.string) {
                str_release(vm_get_str(vm), rec_val.as.string);
            }
            if (rec_val.type != VAL_NUMBER) {
                err.code = 13;
                err.message = "Type mismatch";
                return err;
            }
            record = (long)rec_val.as.number;
            tok = lex_peek(lex);
        }
    }

    char var_name[256] = {0};
    if (tok.type == TOK_COMMA) {
        lex_next(lex);
        tok = lex_peek(lex);
        if (tok.type == TOK_IDENT) {
            size_t vlen = (tok.length < sizeof(var_name) - 1) ? tok.length : sizeof(var_name) - 1;
            memcpy(var_name, tok.start, vlen);
            var_name[vlen] = '\0';
            lex_next(lex);
        }
    }

    FileContext *fctx = vm_get_file(vm);
    if (!file_is_open(fctx, channel)) {
        err.code = 52; err.message = "Bad file number";
        return err;
    }

    FILE *fp = file_get_handle(fctx, channel);
    VDev *dev = file_get_vdev(fctx, channel);
    if (record > 0) {
        file_seek(fctx, channel, record);
    }

    int rlen = file_get_record_len(fctx, channel);
    char *buf = (char *)calloc(1, rlen + 1);
    if (!buf) {
        err.code = 7; err.message = "Out of memory";
        return err;
    }

    BppError over_err = file_check_overlap(fctx, channel, record, record + rlen - 1);
    if (over_err.code != 0) {
        free(buf);
        return over_err;
    }

    size_t bytes_read = 0;
    if (dev) {
        int read_res = vdev_read(dev, buf, rlen);
        bytes_read = (read_res > 0) ? (size_t)read_res : 0;
    } else {
        bytes_read = fread(buf, 1, rlen, fp);
    }
    buf[bytes_read] = '\0';

    if (var_name[0] != '\0') {
        VariableContext *vctx = vm_get_var(vm);
        BValue *var_val = var_lookup(vctx, var_name, true);
        if (!var_val) {
            err.code = 2; err.message = "Variable not declared";
            free(buf);
            return err;
        }
        bool is_string = false;
        size_t nlen = strlen(var_name);
        if (nlen > 0 && var_name[nlen-1] == '$') is_string = true;
        
        if (is_string) {
            var_val->type = VAL_STRING;
            if (var_val->as.string) str_release(vm_get_str(vm), var_val->as.string);
            var_val->as.string = str_create(vm_get_str(vm), buf, (int)bytes_read);
        } else {
            var_val->type = VAL_NUMBER;
            var_val->as.number = 0.0;
            if (bytes_read > 0) {
                size_t cpy_len = (bytes_read < sizeof(double)) ? bytes_read : sizeof(double);
                memcpy(&var_val->as.number, buf, cpy_len);
            }
        }
    } else {
        char *rb = (char *)file_get_record_buffer(fctx, channel);
        if (rb) memcpy(rb, buf, bytes_read);
    }

    free(buf);
    return err;
}

/**
 * @brief PUT #channel, [record_number], variable
 */
BppError stmt_put_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_HASH) {
        lex_next(lex);
    }

    BValue ch_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (ch_val.type == VAL_STRING && ch_val.as.string) {
        str_release(vm_get_str(vm), ch_val.as.string);
    }
    if (ch_val.type != VAL_NUMBER) {
        err.code = 13;
        err.message = "Type mismatch";
        return err;
    }
    int channel = (int)ch_val.as.number;

    tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex);
    }

    long record = -1;
    tok = lex_peek(lex);
    if (tok.type != TOK_EOF && tok.type != TOK_EOL && tok.type != TOK_COMMA) {
        if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_ELSE) {
            /* Stop parsing */
        } else {
            BValue rec_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (rec_val.type == VAL_STRING && rec_val.as.string) {
                str_release(vm_get_str(vm), rec_val.as.string);
            }
            if (rec_val.type != VAL_NUMBER) {
                err.code = 13;
                err.message = "Type mismatch";
                return err;
            }
            record = (long)rec_val.as.number;
            tok = lex_peek(lex);
        }
    }

    char var_name[256] = {0};
    if (tok.type == TOK_COMMA) {
        lex_next(lex);
        tok = lex_peek(lex);
        if (tok.type == TOK_IDENT) {
            size_t vlen = (tok.length < sizeof(var_name) - 1) ? tok.length : sizeof(var_name) - 1;
            memcpy(var_name, tok.start, vlen);
            var_name[vlen] = '\0';
            lex_next(lex);
        }
    }

    FileContext *fctx = vm_get_file(vm);
    if (!file_is_open(fctx, channel)) {
        err.code = 52; err.message = "Bad file number";
        return err;
    }

    FILE *fp = file_get_handle(fctx, channel);
    VDev *dev = file_get_vdev(fctx, channel);
    if (record > 0) {
        file_seek(fctx, channel, record);
    }

    int rlen = file_get_record_len(fctx, channel);
    char *buf = (char *)calloc(1, rlen);
    if (!buf) {
        err.code = 7; err.message = "Out of memory";
        return err;
    }
    memset(buf, ' ', rlen);

    BppError over_err = file_check_overlap(fctx, channel, record, record + rlen - 1);
    if (over_err.code != 0) {
        free(buf);
        return over_err;
    }

    if (var_name[0] != '\0') {
        VariableContext *vctx = vm_get_var(vm);
        BValue *var_val = var_lookup(vctx, var_name, false);
        if (!var_val) {
            err.code = 2; err.message = "Variable undefined";
            free(buf);
            return err;
        }
        if (var_val->type == VAL_STRING) {
            const char *data = str_data(var_val->as.string);
            size_t dlen = strlen(data);
            if (dlen > (size_t)rlen) dlen = (size_t)rlen;
            memcpy(buf, data, dlen);
        } else {
            size_t cpy_len = (sizeof(double) < (size_t)rlen) ? sizeof(double) : (size_t)rlen;
            memcpy(buf, &var_val->as.number, cpy_len);
        }
    } else {
        char *rb = (char *)file_get_record_buffer(fctx, channel);
        if (rb) memcpy(buf, rb, rlen);
    }

    if (dev) {
        vdev_write(dev, buf, rlen);
    } else {
        fwrite(buf, 1, rlen, fp);
    }
    file_flush(fctx, channel);

    free(buf);
    return err;
}

/**
 * @brief SEEK #channel, position
 */
BppError stmt_seek_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_HASH) {
        lex_next(lex);
    }

    BValue ch_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (ch_val.type == VAL_STRING && ch_val.as.string) {
        str_release(vm_get_str(vm), ch_val.as.string);
    }
    if (ch_val.type != VAL_NUMBER) {
        err.code = 13;
        err.message = "Type mismatch";
        return err;
    }
    int channel = (int)ch_val.as.number;

    tok = lex_next(lex);
    if (tok.type != TOK_COMMA) {
        err.code = 2; err.message = "Expected ',' after channel in SEEK";
        return err;
    }

    BValue pos_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (pos_val.type == VAL_STRING && pos_val.as.string) {
        str_release(vm_get_str(vm), pos_val.as.string);
    }
    if (pos_val.type != VAL_NUMBER) {
        err.code = 13;
        err.message = "Type mismatch";
        return err;
    }
    long position = (long)pos_val.as.number;

    file_seek(vm_get_file(vm), channel, position);
    return err;
}

/**
 * @brief Dispatch logic called from print/write statement handler if token is '#'
 */
BppError stmt_file_print_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BValue ch_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (ch_val.type == VAL_STRING && ch_val.as.string) {
        str_release(vm_get_str(vm), ch_val.as.string);
    }
    if (ch_val.type != VAL_NUMBER) {
        err.code = 13;
        err.message = "Type mismatch";
        return err;
    }
    int channel = (int)ch_val.as.number;

    FileContext *fctx = vm_get_file(vm);
    if (!file_is_open(fctx, channel)) {
        err.code = 52; err.message = "Bad file number";
        return err;
    }
    /* Consume separator after channel, e.g. PRINT #1, "hello" */
    BppToken separator = lex_next(lex);
    if (separator.type != TOK_COMMA && separator.type != TOK_SEMICOLON) {
        err.code = 2; err.message = "Expected ',' or ';' after file channel";
        return err;
    }

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_USING) {
        lex_next(lex); /* Consume 'USING' */
        print_using_internal(vm, lex, channel);
        return err;
    }

    if (tok.type == TOK_EOL || tok.type == TOK_EOF) {
        file_putc(fctx, channel, '\n');
        file_flush(fctx, channel);
        return err;
    }

    size_t col = 0;
    while (true) {
        tok = lex_peek(lex);
        if (tok.type == TOK_IDENT && tok.length == 3 && (strncasecmp(tok.start, "TAB", 3) == 0 || strncasecmp(tok.start, "SPC", 3) == 0)) {
            bool is_tab = (strncasecmp(tok.start, "TAB", 3) == 0);
            lex_next(lex); /* Consume TAB/SPC */
            BppToken open_paren = lex_peek(lex);
            if (open_paren.type != TOK_LPAREN) {
                err.code = 2; err.message = "Expected '(' after TAB/SPC";
                return err;
            }
            lex_next(lex); /* Consume '(' */
            BValue arg_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            BppToken close_paren = lex_peek(lex);
            if (close_paren.type != TOK_RPAREN) {
                err.code = 2; err.message = "Expected ')' after TAB/SPC argument";
                return err;
            }
            lex_next(lex); /* Consume ')' */

            int n = (int)arg_val.as.number;
            if (is_tab) {
                int spaces = n - (int)col;
                if (spaces > 0) {
                    for (int i = 0; i < spaces; ++i) {
                        file_putc(fctx, channel, ' ');
                    }
                    col += spaces;
                }
            } else {
                if (n > 0) {
                    for (int i = 0; i < n; ++i) {
                        file_putc(fctx, channel, ' ');
                    }
                    col += n;
                }
            }
        } else {
            BValue val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;

            if (val.type == VAL_STRING) {
                const char *s = str_data(val.as.string);
                file_puts(fctx, channel, s);
                col += strlen(s);
                str_release(vm_get_str(vm), val.as.string);
            } else {
                char buf[64];
                num_format_serialize(buf, sizeof(buf), val.as.number);
                file_puts(fctx, channel, buf);
                col += strlen(buf);
            }
        }

        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA) {
            lex_next(lex);
            file_putc(fctx, channel, '\t');
            col += 8 - (col % 8);
        } else if (tok.type == TOK_SEMICOLON) {
            lex_next(lex);
        } else {
            /* End of print list, add newline */
            file_putc(fctx, channel, '\n');
            break;
        }

        /* Check for trailing newline suppresses */
        tok = lex_peek(lex);
        if (tok.type == TOK_EOL || tok.type == TOK_EOF) {
            break;
        }
    }

    file_flush(fctx, channel);
    return err;
}

/**
 * @brief Dispatch logic called from input statement handler if token is '#'
 * Syntax: INPUT #channel, var1, var2, ...
 */
BppError stmt_file_input_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BValue ch_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (ch_val.type == VAL_STRING && ch_val.as.string) {
        str_release(vm_get_str(vm), ch_val.as.string);
    }
    if (ch_val.type != VAL_NUMBER) {
        err.code = 13;
        err.message = "Type mismatch";
        return err;
    }
    int channel = (int)ch_val.as.number;

    FileContext *fctx = vm_get_file(vm);
    if (!file_is_open(fctx, channel)) {
        err.code = 52; err.message = "Bad file number";
        return err;
    }
    /* Consume separator after channel, e.g. INPUT #1, A */
    BppToken tok = lex_next(lex);
    if (tok.type != TOK_COMMA) {
        err.code = 2; err.message = "Expected ',' after file channel in INPUT #";
        return err;
    }

    VariableContext *vctx = vm_get_var(vm);

    while (true) {
        tok = lex_next(lex);
        if (tok.type != TOK_IDENT) {
            err.code = 2; err.message = "Expected variable name in INPUT #";
            return err;
        }

        char var_name[256];
        size_t vlen = (tok.length < sizeof(var_name) - 1) ? tok.length : sizeof(var_name) - 1;
        memcpy(var_name, tok.start, vlen);
        var_name[vlen] = '\0';

        BValue *var_val = var_lookup(vctx, var_name, true);
        if (!var_val) {
            err.code = 2; err.message = "Variable not declared (OPTION EXPLICIT)";
            return err;
        }

        /* Read next field from stream */
        char field[512];
        if (!read_field(fctx, channel, field, sizeof(field))) {
            err.code = 62; err.message = "Input past end of file";
            return err;
        }

        if (var_val->type == VAL_STRING) {
            if (var_val->as.string) str_release(vm_get_str(vm), var_val->as.string);
            var_val->as.string = str_create(vm_get_str(vm), field, (int)strlen(field));
        } else {
            var_val->type = VAL_NUMBER;
            var_val->as.number = strtod(field, NULL);
        }

        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA) {
            lex_next(lex);
        } else {
            break;
        }
    }

    return err;
}

/**
 * @brief LINE INPUT [#channel,] stringvar$
 */
BppError stmt_line_input_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    VDevContext *vdev = vm_get_vdev(vm);
    FileContext *fctx = vm_get_file(vm);
    VariableContext *vctx = vm_get_var(vm);

    BppToken tok = lex_peek(lex);

    /* Check prompt or channel */
    int channel = -1;
    const char *prompt = NULL;
    size_t prompt_len = 0;

    if (tok.type == TOK_HASH) {
        lex_next(lex); /* Consume '#' */
        BValue ch_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        channel = (int)ch_val.as.number;

        tok = lex_next(lex);
        if (tok.type != TOK_COMMA) {
            err.code = 2; err.message = "Expected ',' after channel number";
            return err;
        }
    } else if (tok.type == TOK_STRING) {
        lex_next(lex);
        prompt = tok.as.string;
        prompt_len = tok.length;

        tok = lex_next(lex);
        if (tok.type != TOK_SEMICOLON && tok.type != TOK_COMMA) {
            err.code = 2; err.message = "Expected ';' or ',' after prompt string";
            return err;
        }
    }

    /* Target Variable */
    tok = lex_next(lex);
    if (tok.type != TOK_IDENT) {
        err.code = 2; err.message = "Expected variable name in LINE INPUT";
        return err;
    }

    char var_name[256];
    size_t vlen = (tok.length < sizeof(var_name) - 1) ? tok.length : sizeof(var_name) - 1;
    memcpy(var_name, tok.start, vlen);
    var_name[vlen] = '\0';

    BValue *var_val = var_lookup(vctx, var_name, true);
    if (!var_val) {
        err.code = 2; err.message = "Variable not declared (OPTION EXPLICIT)";
        return err;
    }
    if (var_val->type != VAL_STRING) {
        err.code = 13; err.message = "LINE INPUT requires a string variable";
        return err;
    }

    char input_buf[1024] = {0};

    if (channel != -1) {
        /* Read line from file channel */
        if (!file_is_open(fctx, channel)) {
            err.code = 52; err.message = "Bad file number";
            return err;
        }
        if (!file_gets(fctx, channel, input_buf, sizeof(input_buf))) {
            err.code = 62; err.message = "Input past end of file";
            return err;
        }
        /* Strip trailing newlines */
        size_t len = strlen(input_buf);
        while (len > 0 && (input_buf[len - 1] == '\n' || input_buf[len - 1] == '\r')) {
            input_buf[len - 1] = '\0';
            len--;
        }
    } else {
        /* Read line from console gets */
        if (prompt) {
            char pr_buf[512];
            size_t copy_len = (prompt_len < sizeof(pr_buf) - 1) ? prompt_len : sizeof(pr_buf) - 1;
            memcpy(pr_buf, prompt, copy_len);
            pr_buf[copy_len] = '\0';
            vdev_puts(vdev, pr_buf);
        }
        VDev *con = vdev_get(vdev, "CON:");
        if (con && con->ops.flush) con->ops.flush(con);

        if (!get_console_line(vm, input_buf, sizeof(input_buf))) {
            err.code = 5; err.message = "Failed to read input from console";
            return err;
        }
    }

    /* Assign to string variable */
    if (var_val->as.string) {
        str_release(vm_get_str(vm), var_val->as.string);
    }
    var_val->as.string = str_create(vm_get_str(vm), input_buf, (int)strlen(input_buf));

    return err;
}

#ifdef BPP_LITE_BUILD
BppError stmt_line_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_INPUT) {
        lex_next(lex); /* Consume INPUT */
        return stmt_line_input_handler(vm, lex);
    }
    err.code = 2;
    err.message = "Graphics statements not supported in this edition";
    return err;
}
#endif

/**
 * @brief IOCTL #channel, cmd[, arg]
 */
BppError stmt_ioctl_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_HASH) {
        lex_next(lex);
    }

    BValue ch_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    int channel = (int)ch_val.as.number;

    tok = lex_next(lex);
    if (tok.type != TOK_COMMA) {
        err.code = 2; err.message = "Expected ',' after channel in IOCTL";
        return err;
    }

    BValue cmd_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    int cmd = (int)cmd_val.as.number;

    int val = 0;
    void *arg_ptr = NULL;

    tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex); /* Consume ',' */
        BValue arg_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (arg_val.type == VAL_STRING) {
            arg_ptr = (void *)str_data(arg_val.as.string);
        } else {
            val = (int)arg_val.as.number;
            arg_ptr = &val;
        }
    }

    FileContext *fctx = vm_get_file(vm);
    if (!file_is_open(fctx, channel)) {
        err.code = 52; err.message = "Bad file number";
        return err;
    }

    VDev *dev = file_get_vdev(fctx, channel);
    if (!dev) {
        err.code = 57; err.message = "Device I/O error: channel is not a device";
        return err;
    }

    int res = vdev_ioctl(dev, cmd, arg_ptr);
    if (res < 0) {
        err.code = 57; err.message = "Device I/O error: IOCTL failed";
        return err;
    }

    return err;
}

#include "platform/platform.h"

/**
 * @brief LOCK [#]channel [, [record] [TO record]]
 */
BppError stmt_lock_handler(VMContext *vm, LexerContext *lex) {
    BppError err; memset(&err, 0, sizeof(err));
    if (security_check(SECOP_FILE_MGMT, 0) != 0) {
        err.code = 70; err.message = "Permission denied";
        return err;
    }
    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_HASH) lex_next(lex);
    BValue ch_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    int channel = (int)ch_val.as.number;
    FILE *fp = file_get_handle(vm_get_file(vm), channel);
    if (!fp) {
        err.code = 52; err.message = "Bad file number";
        return err;
    }
    long start = 1;
    long end = 0x7FFFFFFF;

    if (lex_peek(lex).type == TOK_COMMA) {
        lex_next(lex);
        BppToken p = lex_peek(lex);
        if (!(p.type == TOK_KEYWORD && p.as.keyword == KW_TO) && p.type != TOK_EOL && p.type != TOK_EOF) {
            BValue r1 = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (r1.type == VAL_STRING) { str_release(vm_get_str(vm), r1.as.string); }
            else { start = (long)r1.as.number; end = start; }
        }
        p = lex_peek(lex);
        if (p.type == TOK_KEYWORD && p.as.keyword == KW_TO) {
            lex_next(lex);
            p = lex_peek(lex);
            if (p.type != TOK_EOL && p.type != TOK_EOF) {
                BValue r2 = eval_expression(vm, lex, &err);
                if (err.code != 0) return err;
                if (r2.type == VAL_STRING) { str_release(vm_get_str(vm), r2.as.string); }
                else { end = (long)r2.as.number; }
            }
        }
    }
    
    err = file_lock_range(vm_get_file(vm), channel, start, end);
    return err;
}

/**
 * @brief UNLOCK [#]channel [, [record] [TO record]]
 */
BppError stmt_unlock_handler(VMContext *vm, LexerContext *lex) {
    BppError err; memset(&err, 0, sizeof(err));
    if (security_check(SECOP_FILE_MGMT, 0) != 0) {
        err.code = 70; err.message = "Permission denied";
        return err;
    }
    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_HASH) lex_next(lex);
    BValue ch_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    int channel = (int)ch_val.as.number;
    FILE *fp = file_get_handle(vm_get_file(vm), channel);
    if (!fp) {
        err.code = 52; err.message = "Bad file number";
        return err;
    }
    long start = 1;
    long end = 0x7FFFFFFF;

    if (lex_peek(lex).type == TOK_COMMA) {
        lex_next(lex);
        BppToken p = lex_peek(lex);
        if (!(p.type == TOK_KEYWORD && p.as.keyword == KW_TO) && p.type != TOK_EOL && p.type != TOK_EOF) {
            BValue r1 = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (r1.type == VAL_STRING) { str_release(vm_get_str(vm), r1.as.string); }
            else { start = (long)r1.as.number; end = start; }
        }
        p = lex_peek(lex);
        if (p.type == TOK_KEYWORD && p.as.keyword == KW_TO) {
            lex_next(lex);
            p = lex_peek(lex);
            if (p.type != TOK_EOL && p.type != TOK_EOF) {
                BValue r2 = eval_expression(vm, lex, &err);
                if (err.code != 0) return err;
                if (r2.type == VAL_STRING) { str_release(vm_get_str(vm), r2.as.string); }
                else { end = (long)r2.as.number; }
            }
        }
    }
    
    err = file_unlock_range(vm_get_file(vm), channel, start, end);
    return err;
}

/**
 * @brief FIELD [#]channel, width AS string_var [, width AS string_var...]
 */
BppError stmt_field_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_HASH) {
        lex_next(lex);
    }

    BValue ch_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (ch_val.type == VAL_STRING) {
        err.code = 13; err.message = "Channel must be numeric";
        str_release(vm_get_str(vm), ch_val.as.string);
        return err;
    }
    int channel = (int)ch_val.as.number;
    if (channel < 1 || channel > BPP_MAX_OPEN_FILES) {
        err.code = 52; err.message = "Bad file number";
        return err;
    }
    
    if (file_get_mode(vm_get_file(vm), channel) != FILE_MODE_RANDOM) {
        err.code = 54; err.message = "Bad file mode (must be RANDOM)";
        return err;
    }
    
    unsigned char *record_buffer = file_get_record_buffer(vm_get_file(vm), channel);
    if (!record_buffer) {
        err.code = 52; err.message = "Bad file number (not open)";
        return err;
    }

    tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex);
    } else {
        err.code = 2; err.message = "Expected ',' after channel";
        return err;
    }

    int current_offset = 0;
    while (true) {
        /* Read width */
        BValue width_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (width_val.type == VAL_STRING) {
            err.code = 13; err.message = "Width must be numeric";
            str_release(vm_get_str(vm), width_val.as.string);
            return err;
        }
        int width = (int)width_val.as.number;
        if (width < 0) width = 0;

        /* Read AS */
        tok = lex_next(lex);
        if (tok.type != TOK_KEYWORD || tok.as.keyword != KW_AS) {
            err.code = 2; err.message = "Expected AS in FIELD statement";
            return err;
        }

        /* Read variable name */
        tok = lex_next(lex);
        if (tok.type != TOK_IDENT) {
            err.code = 2; err.message = "Expected string variable in FIELD";
            return err;
        }
        char var_name[256];
        size_t vlen = (tok.length < sizeof(var_name) - 1) ? tok.length : sizeof(var_name) - 1;
        memcpy(var_name, tok.start, vlen);
        var_name[vlen] = '\0';
        for (size_t i = 0; i < vlen; i++) var_name[i] = (char)toupper((unsigned char)var_name[i]);

        if (var_name[vlen - 1] != '$') {
            err.code = 13; err.message = "Type mismatch: FIELD expects string variable";
            return err;
        }

        /* Bind to VAL_FIELD_STRING */
        BValue field_val;
        field_val.type = VAL_FIELD_STRING;
        field_val.as.field_str.channel = channel;
        field_val.as.field_str.offset = current_offset;
        field_val.as.field_str.length = width;
        var_assign(vm_get_var(vm), var_name, field_val);

        current_offset += width;

        /* Check for comma */
        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA) {
            lex_next(lex);
        } else {
            break;
        }
    }

    return err;
}
