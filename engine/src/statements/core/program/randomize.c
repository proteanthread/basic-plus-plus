// FILENAME: randomize.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (arrays.h, arrays.c, ctype.h, ctype.c, file.h, file.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, stmt.h, string.c, time.h, time.c)
// NEEDS: libkernel (vdev.h, vdev.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the RANDOMIZE statement in BASIC++.
//
// ---- Includes ----

#include "stmt/stmt.h"
#include "runtime/language_descriptor.h"
#include "device/vdev.h"
#include "eval/eval.h"
#include "runtime/file.h"
#include "runtime/arrays.h"
#include "platform/platform.h"
#include "runtime/format/snprintf.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/ctype/ctype.h"
#include "eval/functions/math/random/rnd.h"

static const LangDesc g_randomize_desc = {
    .name = "RANDOMIZE",
    .category = "System Control",
    .syntax = "RANDOMIZE [seed% | TIMER | GET[#] ch | INPUT[#] ch | READ [count] | arr[()] | seed$]",
    .description = "Reseeds the random number generator using a numeric seed, system timer, channel byte/line, DATA items, array elements, or string hash.",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_STATEMENT
};

BppError stmt_randomize_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);

    // If no argument supplied, prompt the user interactively
    if (tok.type == TOK_EOL || tok.type == TOK_EOF || (tok.type == TOK_KEYWORD && tok.as.keyword == KW_ELSE)) {
        VDevContext *con_dev = vm_get_vdev(vm);
        if (con_dev) {
            vdev_puts(con_dev, "Random number seed (-32768 to 32767)? ");
        }

        char input_buf[64] = "";
        if (con_dev) {
            vdev_gets(con_dev, input_buf, sizeof(input_buf));
        }

        // Strip trailing newlines
        size_t len = runtime_strlen(input_buf);
        while (len > 0 && (input_buf[len - 1] == '\n' || input_buf[len - 1] == '\r')) {
            input_buf[len - 1] = '\0';
            len--;
        }

        long seed = 0;
        const char *sp = input_buf;
        while (*sp == ' ' || *sp == '\t') sp++;
        bool neg = false;
        if (*sp == '-') { neg = true; sp++; }
        else if (*sp == '+') { sp++; }
        while (*sp >= '0' && *sp <= '9') {
            seed = seed * 10 + (*sp - '0');
            sp++;
        }
        if (neg) seed = -seed;
        func_rnd_set_seed((uint64_t)seed);
        return err;
    }

    // Check for RANDOMIZE TIMER
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_TIMER) {
        lex_next(lex); // Consume TIMER
        func_rnd_set_seed((uint64_t)(platform_get_timer() * 1000.0) ^ (uint64_t)platform_get_uptime());
        return err;
    }

    // Check for RANDOMIZE GET# channel or RANDOMIZE GET channel
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_GET) {
        lex_next(lex); // Consume GET
        BppToken next = lex_peek(lex);
        if (next.type == TOK_HASH) {
            lex_next(lex);
        }
        BValue ch_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        int channel = (int)ch_val.as.number;

        int byte = file_getc(vm_get_file(vm), channel);
        if (byte == -1) byte = 0;
        func_rnd_set_seed((uint64_t)byte);
        return err;
    }

    // Check for RANDOMIZE INPUT# channel or RANDOMIZE INPUT channel
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_INPUT) {
        lex_next(lex); // Consume INPUT
        BppToken next = lex_peek(lex);
        if (next.type == TOK_HASH) {
            lex_next(lex);
        }
        BValue ch_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        int channel = (int)ch_val.as.number;

        char line_buf[256] = "";
        file_gets(vm_get_file(vm), channel, line_buf, sizeof(line_buf));

        unsigned int hash = 5381;
        for (int i = 0; line_buf[i]; i++) {
            hash = ((hash << 5) + hash) + (unsigned char)line_buf[i];
        }
        func_rnd_set_seed(hash);
        return err;
    }

    // Check for RANDOMIZE READ
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_READ) {
        lex_next(lex); // Consume READ
        
        // Optional count argument
        int count = 1;
        BppToken next = lex_peek(lex);
        if (next.type != TOK_EOL && next.type != TOK_EOF && next.type != TOK_KEYWORD) {
            BValue cnt_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            count = (int)cnt_val.as.number;
            if (count < 1) count = 1;
        }

        if (vm_get_data_count(vm) == 0 && vm_get_data_items(vm) == NULL) {
            vm_build_data_table(vm);
        }

        unsigned int hash = 5381;
        int items_read = 0;

        while (items_read < count) {
            int data_ptr = vm_get_data_ptr(vm);
            int data_count = vm_get_data_count(vm);
            BppDataPosition *data_items = vm_get_data_items(vm);

            if (data_ptr >= data_count || !data_items) {
                break;
            }

            BppDataPosition dp = data_items[data_ptr];
            vm_set_data_ptr(vm, data_ptr + 1);

            const char *p_data = dp.pos;
            while (*p_data && *p_data != ',' && *p_data != ':' && *p_data != '\n' && *p_data != '\r') {
                hash = ((hash << 5) + hash) + (unsigned char)*p_data;
                p_data++;
            }
            items_read++;
        }

        func_rnd_set_seed(hash);
        return err;
    }

    // Check for Array shuffle syntax (e.g. RANDOMIZE arr or RANDOMIZE arr())
    if (tok.type == TOK_IDENT) {
        char arr_name[256];
        size_t clen = (tok.length < sizeof(arr_name) - 1) ? tok.length : sizeof(arr_name) - 1;
        runtime_memcpy(arr_name, tok.start, clen);
        arr_name[clen] = '\0';

        if (arr_exists(vm_get_arr(vm), arr_name)) {
            lex_next(lex); // Consume array name
            BppToken next = lex_peek(lex);
            if (next.type == TOK_LPAREN) {
                lex_next(lex);
                next = lex_peek(lex);
                if (next.type == TOK_RPAREN) {
                    lex_next(lex);
                } else {
                    err.code = 2; err.message = "Expected ')' in RANDOMIZE array syntax";
                    return err;
                }
            }

            int total_size = 0;
            BValue *elements = arr_get_flat_elements(vm_get_arr(vm), arr_name, &total_size);
            if (elements && total_size > 0) {
                for (int i = total_size - 1; i > 0; i--) {
                    int j = (int)(func_rnd_eval(vm, "RND", 0, NULL, &err).as.number * (double)(i + 1));
                    if (j < 0) j = 0;
                    if (j > i) j = i;
                    BValue temp = elements[i];
                    elements[i] = elements[j];
                    elements[j] = temp;
                }
            }
            return err;
        }
    }

    // Default case: RANDOMIZE expression
    BValue seed_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (seed_val.type == VAL_STRING) {
        const char *s = str_data(seed_val.as.string);
        unsigned int hash = 5381;
        for (int i = 0; s[i]; i++) {
            hash = ((hash << 5) + hash) + (unsigned char)s[i];
        }
        func_rnd_set_seed(hash);
        str_release(vm_get_str(vm), seed_val.as.string);
    } else {
        long seed = (long)seed_val.as.number;
        func_rnd_set_seed((uint64_t)seed);
    }

    return err;
}

void stmt_randomize_register(void) {
    lang_desc_register(&g_randomize_desc);
}
