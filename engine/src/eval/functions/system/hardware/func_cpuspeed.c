// FILENAME: func_cpuspeed.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine
// Evaluator implementation for CPUSPEED and CPUSPEED$.

#include "eval/functions/system/hardware/func_cpuspeed.h"
#include "hardware/speed_db.h"
#include "hardware/speed_emulate.h"
#include "platform/plat_hw_speed.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "runtime/map.h"
#include "eval/eval.h"
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

static const LangDesc g_cpuspeed_desc = {
    .name = "CPUSPEED",
    .category = "Hardware & Peripherals",
    .syntax = "CPUSPEED | CPUSPEED(\"model\") | CPUSPEED[channel] | CPUSPEED{config}",
    .description = "Returns host CPU speed, relative execution ratios vs vintage systems, and controls emulation.",
    .error_summary = "Error 5: Illegal Function Call",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

static const LangDesc g_cpuspeed_str_desc = {
    .name = "CPUSPEED$",
    .category = "Hardware & Peripherals",
    .syntax = "CPUSPEED$ | CPUSPEED$(\"model\")",
    .description = "Returns formatted host or target CPU speed with unit suffix (e.g. \"3.40 GHz\", \"4.77 MHz\").",
    .error_summary = "Error 5: Illegal Function Call",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_cpuspeed_register(void) {
    lang_desc_register(&g_cpuspeed_desc);
    lang_desc_register(&g_cpuspeed_str_desc);
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

static BppMap *build_spec_map(VMContext *vm, const LegacySystemSpec *spec, double host_hz) {
    BppMap *m = map_create();
    if (!m || !spec) return m;

    void *str_ctx = vm_get_str(vm);
    BValue v;
    v = make_string_val(vm, spec->model); map_set(str_ctx, m, "model", v); str_release(str_ctx, v.as.string);
    map_set(str_ctx, m, "year", make_num_val((double)spec->year));
    map_set(str_ctx, m, "cpu_hz", make_num_val(spec->cpu_hz));
    map_set(str_ctx, m, "cpu_mhz", make_num_val(spec->cpu_mhz));
    v = make_string_val(vm, spec->cpu_arch); map_set(str_ctx, m, "cpu_arch", v); str_release(str_ctx, v.as.string);
    map_set(str_ctx, m, "bus_hz", make_num_val(spec->bus_hz));
    map_set(str_ctx, m, "bus_mhz", make_num_val(spec->bus_mhz));
    v = make_string_val(vm, spec->bus_type); map_set(str_ctx, m, "bus_type", v); str_release(str_ctx, v.as.string);
    map_set(str_ctx, m, "ratio", make_num_val(speed_db_calc_ratio(host_hz, spec->cpu_hz)));
    return m;
}

static BppMap *build_host_map(VMContext *vm) {
    BppMap *m = map_create();
    if (!m) return m;

    void *str_ctx = vm_get_str(vm);
    double cpu_hz = plat_hw_get_cpu_hz();
    map_set(str_ctx, m, "cpu_hz", make_num_val(cpu_hz));
    map_set(str_ctx, m, "cpu_mhz", make_num_val(cpu_hz / 1000000.0));
    map_set(str_ctx, m, "bus_hz", make_num_val(plat_hw_get_bus_hz()));
    map_set(str_ctx, m, "mem_hz", make_num_val(plat_hw_get_mem_hz()));
    map_set(str_ctx, m, "gpu_hz", make_num_val(plat_hw_get_gpu_hz()));
    map_set(str_ctx, m, "emulated", make_num_val(speed_emulate_is_active() ? 1.0 : 0.0));
    BValue v = make_string_val(vm, speed_emulate_get_target()); map_set(str_ctx, m, "target", v); str_release(str_ctx, v.as.string);
    map_set(str_ctx, m, "ratio", make_num_val(speed_emulate_get_ratio()));
    return m;
}

BValue func_cpuspeed_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)err;
    bool is_str = (uname && uname[strlen(uname) - 1] == '$');
    double host_cpu_hz = plat_hw_get_cpu_hz();

    // Bare variable: CPUSPEED or CPUSPEED$
    if (arg_count == 0) {
        if (is_str) {
            char buf[32];
            speed_db_format_unit(host_cpu_hz, buf, sizeof(buf));
            return make_string_val(vm, buf);
        }
        return make_num_val(host_cpu_hz);
    }

    // 1 Argument: target model name or numeric MHz
    if (arg_count == 1) {
        if (args[0].type == VAL_STRING && args[0].as.string) {
            const char *target_name = str_data(args[0].as.string);
            const LegacySystemSpec *spec = speed_db_find(target_name);
            if (!spec) {
                if (err) { err->code = 5; err->message = "Illegal function call (Unknown system model)"; }
                return make_num_val(0.0);
            }
            if (is_str) {
                char buf[32];
                speed_db_format_unit(spec->cpu_hz, buf, sizeof(buf));
                return make_string_val(vm, buf);
            }
            return make_num_val(speed_db_calc_ratio(host_cpu_hz, spec->cpu_hz));
        } else if (args[0].type == VAL_NUMBER || args[0].type == VAL_INTEGER) {
            double target_mhz = args[0].as.number;
            double target_hz = target_mhz * 1000000.0;
            if (is_str) {
                char buf[32];
                speed_db_format_unit(target_hz, buf, sizeof(buf));
                return make_string_val(vm, buf);
            }
            return make_num_val(speed_db_calc_ratio(host_cpu_hz, target_hz));
        }
    }

    // 2 Arguments: target model name, domain/metric selector
    if (arg_count >= 2 && args[0].type == VAL_STRING && args[0].as.string) {
        const char *target_name = str_data(args[0].as.string);
        const LegacySystemSpec *spec = speed_db_find(target_name);
        if (!spec) {
            if (err) { err->code = 5; err->message = "Illegal function call (Unknown system model)"; }
            return make_num_val(0.0);
        }

        const char *metric = "";
        int metric_code = -1;
        if (args[1].type == VAL_STRING && args[1].as.string) {
            metric = str_data(args[1].as.string);
        } else if (args[1].type == VAL_NUMBER || args[1].type == VAL_INTEGER) {
            metric_code = (int)args[1].as.number;
        }

        if (case_cmp(metric, "BUS") == 0 || metric_code == 1) {
            return make_num_val(speed_db_calc_ratio(plat_hw_get_bus_hz(), spec->bus_hz));
        }
        if (case_cmp(metric, "RAW") == 0 || metric_code == 0) {
            return make_num_val(spec->cpu_hz);
        }
        if (case_cmp(metric, "MHZ") == 0) {
            return make_num_val(spec->cpu_mhz);
        }
        if (case_cmp(metric, "STR") == 0) {
            char buf[32];
            speed_db_format_unit(spec->cpu_hz, buf, sizeof(buf));
            return make_string_val(vm, buf);
        }
        return make_num_val(speed_db_calc_ratio(host_cpu_hz, spec->cpu_hz));
    }

    return make_num_val(0.0);
}

BValue func_cpuspeed_eval_bracket(VMContext *vm, LexerContext *lex, BppError *err) {
    (void)err;
    char target_str[64] = {0};
    char channel[32] = {0};

    BppToken tok = lex_peek(lex);

    // If first token is string literal (target model)
    if (tok.type == TOK_STRING) {
        tok = lex_next(lex);
        size_t len = tok.length;
        const char *start = tok.start;
        if (len >= 2 && start[0] == '"') { start++; len -= 2; }
        if (len < sizeof(target_str)) {
            memcpy(target_str, start, len);
            target_str[len] = '\0';
        }

        if (lex_peek(lex).type == TOK_COMMA) {
            lex_next(lex); // consume comma
            tok = lex_peek(lex);
        }
    }

    // Next token: identifier channel name or keyword
    if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) {
        tok = lex_next(lex);
        size_t clen = (tok.length < sizeof(channel) - 1) ? tok.length : sizeof(channel) - 1;
        memcpy(channel, tok.start, clen);
        channel[clen] = '\0';
    }

    // Expect closing bracket ']'
    if (lex_peek(lex).type == TOK_RBRACKET) {
        lex_next(lex);
    }

    // Evaluate target-specific slicing: e.g. ["5150", BUS]
    if (target_str[0] != '\0') {
        const LegacySystemSpec *spec = speed_db_find(target_str);
        if (!spec) {
            if (err) { err->code = 5; err->message = "Unknown system model in bracket slice"; }
            return make_num_val(0.0);
        }
        if (case_cmp(channel, "BUS") == 0) return make_num_val(spec->bus_hz);
        if (case_cmp(channel, "CPU") == 0) return make_num_val(spec->cpu_hz);
        if (case_cmp(channel, "MHZ") == 0) return make_num_val(spec->cpu_mhz);
        if (case_cmp(channel, "RATIO") == 0) return make_num_val(speed_db_calc_ratio(plat_hw_get_cpu_hz(), spec->cpu_hz));
        if (case_cmp(channel, "STR") == 0) {
            char buf[32];
            speed_db_format_unit(spec->cpu_hz, buf, sizeof(buf));
            return make_string_val(vm, buf);
        }
        return make_num_val(spec->cpu_hz);
    }

    // Evaluate host channel queries: e.g. [CPU], [BUS], [MEM], [GPU], [MHZ], [GHZ], [STR], [EMULATE]
    if (case_cmp(channel, "CPU") == 0) return make_num_val(plat_hw_get_cpu_hz());
    if (case_cmp(channel, "BUS") == 0) return make_num_val(plat_hw_get_bus_hz());
    if (case_cmp(channel, "MEM") == 0) return make_num_val(plat_hw_get_mem_hz());
    if (case_cmp(channel, "GPU") == 0) return make_num_val(plat_hw_get_gpu_hz());
    if (case_cmp(channel, "MHZ") == 0) return make_num_val(plat_hw_get_cpu_hz() / 1000000.0);
    if (case_cmp(channel, "GHZ") == 0) return make_num_val(plat_hw_get_cpu_hz() / 1000000000.0);
    if (case_cmp(channel, "EMULATE") == 0) return make_num_val(speed_emulate_is_active() ? 1.0 : 0.0);
    if (case_cmp(channel, "STR") == 0) {
        char buf[32];
        speed_db_format_unit(plat_hw_get_cpu_hz(), buf, sizeof(buf));
        return make_string_val(vm, buf);
    }

    return make_num_val(plat_hw_get_cpu_hz());
}

BValue func_cpuspeed_eval_brace(VMContext *vm, LexerContext *lex, BppError *err) {
    (void)err;
    BppToken tok = lex_peek(lex);

    // Empty braces: CPUSPEED{} -> returns host metrics map
    if (tok.type == TOK_RBRACE) {
        lex_next(lex);
        BValue res;
        memset(&res, 0, sizeof(res));
        res.type = VAL_MAP;
        res.as.map = build_host_map(vm);
        return res;
    }

    // Single string literal: CPUSPEED{"5150"} -> returns target spec map
    if (tok.type == TOK_STRING) {
        tok = lex_next(lex);
        size_t len = tok.length;
        const char *start = tok.start;
        if (len >= 2 && start[0] == '"') { start++; len -= 2; }
        char target_name[64];
        if (len < sizeof(target_name)) {
            memcpy(target_name, start, len);
            target_name[len] = '\0';
        } else {
            target_name[0] = '\0';
        }

        if (lex_peek(lex).type == TOK_RBRACE) {
            lex_next(lex);
        }

        const LegacySystemSpec *spec = speed_db_find(target_name);
        BValue res;
        memset(&res, 0, sizeof(res));
        res.type = VAL_MAP;
        res.as.map = build_spec_map(vm, spec, plat_hw_get_cpu_hz());
        return res;
    }

    // Key-value pairs: e.g. {emulate: "5150", io: TRUE} or {add: "C64", cpu: 1022727, ...}
    LegacySystemSpec custom_spec;
    memset(&custom_spec, 0, sizeof(custom_spec));
    bool is_add = false;

    while (lex_peek(lex).type != TOK_RBRACE && lex_peek(lex).type != TOK_EOF) {
        BppToken key_tok = lex_next(lex);
        char key[32] = {0};
        size_t klen = (key_tok.length < sizeof(key) - 1) ? key_tok.length : sizeof(key) - 1;
        memcpy(key, key_tok.start, klen);
        key[klen] = '\0';

        BppToken sep = lex_peek(lex);
        if (sep.type == TOK_EQ || (sep.type == TOK_EOL && sep.start && sep.start[0] == ':')) {
            lex_next(lex); // consume ':' or '='
        }

        BValue val = eval_expression(vm, lex, err);
        if (err && err->code != 0) break;

        if (case_cmp(key, "emulate") == 0) {
            if (val.type == VAL_STRING && val.as.string) {
                if (case_cmp(str_data(val.as.string), "OFF") == 0 || case_cmp(str_data(val.as.string), "NONE") == 0) {
                    speed_emulate_set_active(false);
                } else {
                    speed_emulate_set_target(str_data(val.as.string), speed_emulate_is_io_active());
                }
            } else if (val.type == VAL_NUMBER || val.type == VAL_INTEGER) {
                speed_emulate_set_mhz(val.as.number, speed_emulate_is_io_active());
            }
        } else if (case_cmp(key, "io") == 0) {
            bool io_on = (val.type == VAL_INTEGER || val.type == VAL_NUMBER) && (val.as.number != 0.0);
            if (speed_emulate_is_active()) {
                speed_emulate_set_target(speed_emulate_get_target(), io_on);
            }
        } else if (case_cmp(key, "bus") == 0 && (val.type == VAL_NUMBER || val.type == VAL_INTEGER)) {
            if (is_add) {
                custom_spec.bus_hz = val.as.number;
                custom_spec.bus_mhz = val.as.number / 1000000.0;
            } else {
                plat_hw_set_bus_hz(val.as.number);
            }
        } else if (case_cmp(key, "mem") == 0 && (val.type == VAL_NUMBER || val.type == VAL_INTEGER)) {
            plat_hw_set_mem_hz(val.as.number);
        } else if (case_cmp(key, "gpu") == 0 && (val.type == VAL_NUMBER || val.type == VAL_INTEGER)) {
            plat_hw_set_gpu_hz(val.as.number);
        } else if (case_cmp(key, "add") == 0 && val.type == VAL_STRING && val.as.string) {
            is_add = true;
            strncpy(custom_spec.model, str_data(val.as.string), sizeof(custom_spec.model) - 1);
        } else if (case_cmp(key, "year") == 0 && (val.type == VAL_NUMBER || val.type == VAL_INTEGER)) {
            custom_spec.year = (int)val.as.number;
        } else if (case_cmp(key, "cpu") == 0 && (val.type == VAL_NUMBER || val.type == VAL_INTEGER)) {
            custom_spec.cpu_hz = val.as.number;
            custom_spec.cpu_mhz = val.as.number / 1000000.0;
        } else if (case_cmp(key, "arch") == 0 && val.type == VAL_STRING && val.as.string) {
            strncpy(custom_spec.cpu_arch, str_data(val.as.string), sizeof(custom_spec.cpu_arch) - 1);
        } else if (case_cmp(key, "bus_type") == 0 && val.type == VAL_STRING && val.as.string) {
            strncpy(custom_spec.bus_type, str_data(val.as.string), sizeof(custom_spec.bus_type) - 1);
        }

        if (val.type == VAL_STRING && val.as.string) str_release(vm_get_str(vm), val.as.string);
        else if (val.type == VAL_MAP && val.as.map) map_release(vm_get_str(vm), val.as.map);

        if (lex_peek(lex).type == TOK_COMMA) {
            lex_next(lex); // consume comma
        }
    }

    if (lex_peek(lex).type == TOK_RBRACE) {
        lex_next(lex);
    }

    if (is_add && custom_spec.model[0] != '\0') {
        speed_db_add_custom(&custom_spec);
    }

    BValue res;
    memset(&res, 0, sizeof(res));
    res.type = VAL_MAP;
    res.as.map = build_host_map(vm);
    return res;
}
