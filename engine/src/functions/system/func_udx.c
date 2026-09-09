// FILENAME: func_udx.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, common_reg_funcs.c
// NEEDS: libreg (reg_buffer.h, reg_udx.h, reg_hw.h), runtime (funcreg.h, language_descriptor.h, strops.h, strings.h)
// Implements freestanding UDX, FIFO, LIFO functions in BASIC++.

#include "functions/system/func_udx.h"
#include "reg/reg_buffer.h"
#include "reg/reg_udx.h"
#include "reg/reg_hw.h"
#include "runtime/funcreg.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "runtime/strings.h"

static const LangDesc g_func_udx_desc = {
    .name = "UDX",
    .category = "System & Hardware",
    .syntax = "UDX(op$, [arg1, arg2]) | XCHG(op$, ...)",
    .description = "Polymorphic function for Universal Data Exchange bus and buffer operations.",
    .error_summary = "Error 2: Syntax Error, Error 5: Illegal Function Call",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_FUNCTION
};

static const LangDesc g_func_fifo_desc = {
    .name = "FIFO",
    .category = "System & Hardware",
    .syntax = "FIFO(op$, [val]) | FIFO.PUSH(val) | FIFO.POP()",
    .description = "First-class First In, First Out buffer operation function.",
    .error_summary = "Error 5: Illegal Function Call",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_FUNCTION
};

static const LangDesc g_func_lifo_desc = {
    .name = "LIFO",
    .category = "System & Hardware",
    .syntax = "LIFO(op$, [val]) | LIFO.PUSH(val) | LIFO.POP()",
    .description = "First-class Last In, First Out stack buffer operation function.",
    .error_summary = "Error 5: Illegal Function Call",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_FUNCTION
};

BValue func_udx_eval(BValue *args, int arg_count, void *rt) {
    (void)rt;
    BValue res;
    runtime_memset(&res, 0, sizeof(res));
    res.type = VAL_NUMBER;

    if (arg_count == 0) return res;

    // First argument is operation command string
    if (args[0].type == VAL_STRING && args[0].as.string) {
        const char *op = str_data(args[0].as.string);

        if (runtime_strcasecmp(op, "PUSH") == 0 && arg_count >= 2) {
            const char *target = (arg_count >= 3 && args[2].type == VAL_STRING && args[2].as.string)
                                ? str_data(args[2].as.string) : "FIFO";
            res.type = VAL_INTEGER;
            res.as.number = reg_buffer_push_by_name(target, args[1]) ? -1.0 : 0.0;
            return res;
        }

        if (runtime_strcasecmp(op, "POP") == 0) {
            const char *target = (arg_count >= 2 && args[1].type == VAL_STRING && args[1].as.string)
                                ? str_data(args[1].as.string) : "FIFO";
            reg_buffer_pop_by_name(target, &res);
            return res;
        }

        if (runtime_strcasecmp(op, "COUNT") == 0) {
            const char *target = (arg_count >= 2 && args[1].type == VAL_STRING && args[1].as.string)
                                ? str_data(args[1].as.string) : "FIFO";
            res.type = VAL_INTEGER;
            res.as.number = (double)reg_buffer_count_by_name(target);
            return res;
        }

        if (runtime_strcasecmp(op, "TRANSFER") == 0 && arg_count >= 3) {
            if (args[1].type == VAL_STRING && args[1].as.string &&
                args[2].type == VAL_STRING && args[2].as.string) {
                const char *src = str_data(args[1].as.string);
                const char *dst = str_data(args[2].as.string);
                res.type = VAL_INTEGER;
                res.as.number = reg_buffer_transfer(src, dst) ? -1.0 : 0.0;
                return res;
            }
        }

        if (runtime_strcasecmp(op, "ACC") == 0) {
            char bank = 'A';
            if (arg_count >= 2 && args[1].type == VAL_STRING && args[1].as.string) {
                const char *bs = str_data(args[1].as.string);
                if (bs[0] != '\0') bank = bs[0];
            }
            if (arg_count >= 3) {
                reg_udx_set_acc(bank, args[2]);
            }
            return reg_udx_get_acc(bank);
        }
    }

    return res;
}

BValue func_fifo_eval(BValue *args, int arg_count, void *rt) {
    (void)rt;
    BValue res;
    runtime_memset(&res, 0, sizeof(res));
    res.type = VAL_NUMBER;

    if (arg_count == 0) {
        reg_buffer_pop_by_name("FIFO", &res);
        return res;
    }

    if (args[0].type == VAL_STRING && args[0].as.string) {
        const char *op = str_data(args[0].as.string);
        if (runtime_strcasecmp(op, "PUSH") == 0 && arg_count >= 2) {
            res.type = VAL_INTEGER;
            res.as.number = reg_buffer_push_by_name("FIFO", args[1]) ? -1.0 : 0.0;
            return res;
        }
        if (runtime_strcasecmp(op, "POP") == 0) {
            reg_buffer_pop_by_name("FIFO", &res);
            return res;
        }
        if (runtime_strcasecmp(op, "COUNT") == 0) {
            res.type = VAL_INTEGER;
            res.as.number = (double)reg_buffer_count_by_name("FIFO");
            return res;
        }
        if (runtime_strcasecmp(op, "CLEAR") == 0) {
            reg_buffer_clear(reg_buffer_get_default(REG_DISCIPLINE_FIFO));
            res.type = VAL_INTEGER;
            res.as.number = 0.0;
            return res;
        }
    }

    // Default: push value if single arg
    res.type = VAL_INTEGER;
    res.as.number = reg_buffer_push_by_name("FIFO", args[0]) ? -1.0 : 0.0;
    return res;
}

BValue func_lifo_eval(BValue *args, int arg_count, void *rt) {
    (void)rt;
    BValue res;
    runtime_memset(&res, 0, sizeof(res));
    res.type = VAL_NUMBER;

    if (arg_count == 0) {
        reg_buffer_pop_by_name("LIFO", &res);
        return res;
    }

    if (args[0].type == VAL_STRING && args[0].as.string) {
        const char *op = str_data(args[0].as.string);
        if (runtime_strcasecmp(op, "PUSH") == 0 && arg_count >= 2) {
            res.type = VAL_INTEGER;
            res.as.number = reg_buffer_push_by_name("LIFO", args[1]) ? -1.0 : 0.0;
            return res;
        }
        if (runtime_strcasecmp(op, "POP") == 0) {
            reg_buffer_pop_by_name("LIFO", &res);
            return res;
        }
        if (runtime_strcasecmp(op, "COUNT") == 0) {
            res.type = VAL_INTEGER;
            res.as.number = (double)reg_buffer_count_by_name("LIFO");
            return res;
        }
        if (runtime_strcasecmp(op, "CLEAR") == 0) {
            reg_buffer_clear(reg_buffer_get_default(REG_DISCIPLINE_LIFO));
            res.type = VAL_INTEGER;
            res.as.number = 0.0;
            return res;
        }
    }

    // Default: push value if single arg
    res.type = VAL_INTEGER;
    res.as.number = reg_buffer_push_by_name("LIFO", args[0]) ? -1.0 : 0.0;
    return res;
}

void func_udx_register(void) {
    lang_desc_register(&g_func_udx_desc);
    lang_desc_register(&g_func_fifo_desc);
    lang_desc_register(&g_func_lifo_desc);

    FunctionEntry entry_udx = {
        .name = "UDX",
        .keyword = KW_UDX,
        .category = FCAT_UTIL,
        .ret_type = FRET_ANY,
        .min_args = 0,
        .max_args = 4,
        .safety = FSAFE_STATE,
        .overridable = 0,
        .handler = func_udx_eval,
        .help_text = "Universal Data Exchange bus operation",
        .module_name = "UDX"
    };
    funcreg_register(&entry_udx);

    FunctionEntry entry_xchg = {
        .name = "XCHG",
        .keyword = KW_XCHG,
        .category = FCAT_UTIL,
        .ret_type = FRET_ANY,
        .min_args = 0,
        .max_args = 4,
        .safety = FSAFE_STATE,
        .overridable = 0,
        .handler = func_udx_eval,
        .help_text = "Universal Data Exchange bus operation",
        .module_name = "UDX"
    };
    funcreg_register(&entry_xchg);

    FunctionEntry entry_fifo = {
        .name = "FIFO",
        .keyword = KW_NONE,
        .category = FCAT_UTIL,
        .ret_type = FRET_ANY,
        .min_args = 0,
        .max_args = 2,
        .safety = FSAFE_STATE,
        .overridable = 0,
        .handler = func_fifo_eval,
        .help_text = "First In, First Out buffer operation",
        .module_name = "UDX"
    };
    funcreg_register(&entry_fifo);

    FunctionEntry entry_lifo = {
        .name = "LIFO",
        .keyword = KW_NONE,
        .category = FCAT_UTIL,
        .ret_type = FRET_ANY,
        .min_args = 0,
        .max_args = 2,
        .safety = FSAFE_STATE,
        .overridable = 0,
        .handler = func_lifo_eval,
        .help_text = "Last In, First Out buffer operation",
        .module_name = "UDX"
    };
    funcreg_register(&entry_lifo);
}
