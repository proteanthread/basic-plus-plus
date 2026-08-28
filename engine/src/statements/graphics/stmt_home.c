// FILENAME: stmt_home.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, stmt_home.h, string.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (errors.h, vcon.h, vcon.c, vdev.h, vdev.c)
// Provides runtime implementation for the HOME statement in BASIC++.
//
// ---- Includes ----

#include "statements/graphics/stmt_home.h"
#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "device/vcon.h"
#include "device/vdev.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>
#include <stdio.h>

static const int ansi_fg_map[16] = {30, 34, 32, 36, 31, 35, 33, 37, 90, 94, 92, 96, 91, 95, 93, 97};

void stmt_home_register(void) {
    static const MicroLibMetadata meta = {
        .name = "HOME",
        .category = "Graphics & Display",
        .syntax = "HOME [n]",
        .help_text = "Moves text cursor to top-left corner (1, 1) without clearing text screen, optionally changing text color to n (0-15).",
        .error_codes = "Error 2: Syntax Error, Error 5: Illegal Function Call"
    };
    microlib_register(&meta);
}

BppError stmt_home_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    int color = -1;
    BppToken tok = lex_peek(lex);
    if (tok.type != TOK_EOL && tok.type != TOK_EOF) {
        BValue val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (val.type == VAL_NUMBER) {
            color = (int)val.as.number;
            if (color < 0 || color > 15) {
                err.code = 5;
                err.message = "Illegal Function Call";
                return err;
            }
        } else {
            err.code = 5;
            err.message = "Illegal Function Call";
            return err;
        }
    }

    VConContext *vcon = vm_get_vcon(vm);
    VDevContext *vdev = vm_get_vdev(vm);
    if (vcon) {
        vcon_locate(vcon, 0, 1, 1);
    }

    if (vdev) {
        if (color >= 0 && color <= 15) {
            if (vcon) {
                vcon_set_color(vcon, 0, color, -1);
            }
            vdev_printf(vdev, "\033[%dm\033[H", ansi_fg_map[color]);
        } else {
            vdev_printf(vdev, "\033[H");
        }
    }

    return err;
}
