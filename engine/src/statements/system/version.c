/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file version.c
 * @brief VERSION statement and VER / VER$ evaluator implementation.
 */

#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#define strcasecmp _stricmp
#endif

#include "statements/system/version.h"
#include "types/version.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "memory/memory.h"
#include "device/vdev.h"
#include "runtime/micro_lib_metadata.h"

BppError stmt_version_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_next(lex);
    char ver_buf[32] = {0};

    if (tok.type == TOK_STRING) {
        if (tok.start && tok.length >= 2 && tok.start[0] == '"') {
            size_t copy_len = tok.length - 2;
            if (tok.start[tok.length - 1] != '"') copy_len = tok.length - 1;
            snprintf(ver_buf, sizeof(ver_buf), "%.*s", (int)copy_len, tok.start + 1);
        } else if (tok.start) {
            snprintf(ver_buf, sizeof(ver_buf), "%.*s", (int)tok.length, tok.start);
        }
    } else if (tok.type == TOK_NUMBER) {
        snprintf(ver_buf, sizeof(ver_buf), "%g", tok.as.number);
    } else if (tok.type == TOK_IDENT) {
        snprintf(ver_buf, sizeof(ver_buf), "%.*s", (int)tok.length, tok.start);
    } else {
        err.code = 2; /* Syntax Error */
        err.message = "Expected version string or number after VERSION";
        return err;
    }

    MemoryContext *mem = vm_get_mem(vm);
    if (mem) {
        mem_program_set_version(mem, ver_buf);
    }

    return err;
}

BppError stmt_metadata_handler(VMContext *vm, LexerContext *lex) {
    return stmt_version_handler(vm, lex);
}

BppError stmt_ver_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)lex;

    VDevContext *vdev = vm_get_vdev(vm);
    MemoryContext *mem = vm_get_mem(vm);

    const char *prog_ver = mem ? mem_program_get_version(mem) : "";
    if (!prog_ver || !prog_ver[0]) prog_ver = "(Untagged)";

    vdev_printf(vdev, "BASIC++ Engine Version:  %s\n", VERSION_STRING);
    vdev_printf(vdev, "Program Version Tag:   %s\n", prog_ver);

    return err;
}

BValue func_ver_str_eval(BValue *args, int arg_count, void *rt) {
    VMContext *vm = (VMContext *)rt;
    BValue res;
    memset(&res, 0, sizeof(res));

    MemoryContext *mem = vm_get_mem(vm);
    StringContext *str_ctx = vm_get_str(vm);
    const char *ver_target = "";

    if (arg_count > 0 && args[0].type == VAL_STRING && args[0].as.string) {
        ver_target = str_data(args[0].as.string);
    }

    const char *ret_str = VERSION_STRING;

    if (strcasecmp(ver_target, "PROGRAM") == 0 || strcasecmp(ver_target, "APP") == 0) {
        const char *pv = mem ? mem_program_get_version(mem) : "";
        if (pv && pv[0]) {
            ret_str = pv;
        } else {
            ret_str = "0.0.0";
        }
    }

    res.type = VAL_STRING;
    res.as.string = str_create(str_ctx, ret_str, strlen(ret_str));

    /* Release input string arguments */
    for (int i = 0; i < arg_count; i++) {
        if (args[i].type == VAL_STRING && args[i].as.string) {
            str_release(str_ctx, args[i].as.string);
        }
    }

    return res;
}

#include "runtime/funcreg.h"

void stmt_version_register(void) {
    static const MicroLibMetadata meta = {
        .name = "VERSION",
        .category = "System & Environ",
        .syntax = "VERSION \"1.2.0\" | VER | VER$([target$])",
        .help_text = "Tags or queries active program version metadata and host engine version.",
        .error_codes = "Error 2: Syntax Error"
    };
    microlib_register(&meta);

    FunctionEntry entry = {
        .name = "VER$",
        .keyword = KW_VER_STR,
        .category = FCAT_UTIL,
        .ret_type = FRET_STRING,
        .min_args = 0,
        .max_args = 1,
        .safety = FSAFE_PURE,
        .overridable = 0,
        .handler = func_ver_str_eval,
        .help_text = "Returns engine version or tagged program version string",
        .module_name = "System"
    };
    funcreg_register(&entry);

    FunctionEntry entry_ver = {
        .name = "VER",
        .keyword = KW_VER,
        .category = FCAT_UTIL,
        .ret_type = FRET_STRING,
        .min_args = 0,
        .max_args = 1,
        .safety = FSAFE_PURE,
        .overridable = 0,
        .handler = func_ver_str_eval,
        .help_text = "Returns engine version or tagged program version string",
        .module_name = "System"
    };
    funcreg_register(&entry_ver);
}
