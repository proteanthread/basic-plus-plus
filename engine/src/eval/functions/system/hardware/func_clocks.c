// FILENAME: func_clocks.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine
// Evaluator implementation for CLOCKS and CLOCKS$.

#include "eval/functions/system/hardware/func_clocks.h"
#include "eval/functions/system/hardware/func_cpuspeed.h"
#include "hardware/speed_db.h"
#include "hardware/speed_emulate.h"
#include "platform/plat_hw_speed.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

static int case_cmp(const char *s1, const char *s2) {
    if (!s1 || !s2) return s1 ? 1 : (s2 ? -1 : 0);
    while (*s1 && *s2) {
        int c1 = tolower((unsigned char)*s1);
        int c2 = tolower((unsigned char)*s2);
        if (c1 != c2) return c1 - c2;
        s1++;
        s2++;
    }
    return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
}

static const LangDesc g_clocks_desc = {
    .name = "CLOCKS",
    .category = "Hardware & Peripherals",
    .syntax = "CLOCKS | CLOCKS(\"model\") | CLOCKS[channel] | CLOCKS{config}",
    .description = "Returns hardware cycle counters, multi-domain clocks, and timing metrics.",
    .error_summary = "Error 5: Illegal Function Call",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

static const LangDesc g_clocks_str_desc = {
    .name = "CLOCKS$",
    .category = "Hardware & Peripherals",
    .syntax = "CLOCKS$",
    .description = "Returns multi-domain hardware clock summary string (CPU, Bus, Memory).",
    .error_summary = "Error 5: Illegal Function Call",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_clocks_register(void) {
    lang_desc_register(&g_clocks_desc);
    lang_desc_register(&g_clocks_str_desc);
}

static BValue make_string_val(VMContext *vm, const char *text) {
    BValue res;
    memset(&res, 0, sizeof(res));
    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), text ? text : "", text ? strlen(text) : 0);
    return res;
}

static BValue make_num_val(double num) {
    BValue res;
    memset(&res, 0, sizeof(res));
    res.type = VAL_NUMBER;
    res.as.number = num;
    return res;
}

BValue func_clocks_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)err;
    bool is_str = (uname && uname[strlen(uname) - 1] == '$');

    // Bare CLOCKS or CLOCKS$
    if (arg_count == 0) {
        if (is_str) {
            char cpu_buf[32], bus_buf[32], mem_buf[32];
            speed_db_format_unit(plat_hw_get_cpu_hz(), cpu_buf, sizeof(cpu_buf));
            speed_db_format_unit(plat_hw_get_bus_hz(), bus_buf, sizeof(bus_buf));
            speed_db_format_unit(plat_hw_get_mem_hz(), mem_buf, sizeof(mem_buf));
            char summary[128];
            snprintf(summary, sizeof(summary), "CPU: %s, Bus: %s, Mem: %s", cpu_buf, bus_buf, mem_buf);
            return make_string_val(vm, summary);
        }
        return make_num_val((double)plat_hw_get_cycle_count());
    }

    // With arguments, delegate to func_cpuspeed_eval
    return func_cpuspeed_eval(vm, uname, arg_count, args, err);
}

BValue func_clocks_eval_bracket(VMContext *vm, LexerContext *lex, BppError *err) {
    (void)err;
    char channel[32] = {0};

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) {
        tok = lex_next(lex);
        size_t clen = (tok.length < sizeof(channel) - 1) ? tok.length : sizeof(channel) - 1;
        memcpy(channel, tok.start, clen);
        channel[clen] = '\0';
    }

    if (lex_peek(lex).type == TOK_RBRACKET) {
        lex_next(lex);
    }

    if (case_cmp(channel, "CYCLES") == 0 || case_cmp(channel, "TICKS") == 0) {
        return make_num_val((double)plat_hw_get_cycle_count());
    }
    if (case_cmp(channel, "STATUS") == 0) {
        return make_num_val(speed_emulate_is_active() ? 1.0 : 0.0);
    }
    if (case_cmp(channel, "CPU") == 0) return make_num_val(plat_hw_get_cpu_hz());
    if (case_cmp(channel, "BUS") == 0) return make_num_val(plat_hw_get_bus_hz());
    if (case_cmp(channel, "MEM") == 0) return make_num_val(plat_hw_get_mem_hz());
    if (case_cmp(channel, "GPU") == 0) return make_num_val(plat_hw_get_gpu_hz());

    return make_num_val((double)plat_hw_get_cycle_count());
}

BValue func_clocks_eval_brace(VMContext *vm, LexerContext *lex, BppError *err) {
    return func_cpuspeed_eval_brace(vm, lex, err);
}
