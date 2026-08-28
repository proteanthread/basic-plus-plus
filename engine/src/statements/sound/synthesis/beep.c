// FILENAME: beep.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (beep.h, eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// NEEDS: libkernel (errors.h, security.h, security.c, vdev.h, vdev.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the BEEP statement in BASIC++.
//
// ---- Includes ----

#include "statements/sound/synthesis/beep.h"
#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "security/security.h"
#include "platform/platform.h"
#include "runtime/strings.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

void stmt_beep_register(void) {
    static const MicroLibMetadata meta = {
        .name = "BEEP",
        .category = "Sound & Audio",
        .syntax = "BEEP [count [, delay]]",
        .help_text = "Emits standard 800 Hz speaker beep tones for count repetitions with optional delay in seconds (default 1 beep, 1.0s delay).",
        .error_codes = "Error 2: Syntax Error, Error 5: Illegal Function Call, Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}

BppError stmt_beep_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (security_check(SECOP_VDEV, 0) != 0) {
        err.code = 70;
        err.message = "Permission denied: BEEP blocked by sandbox settings";
        return err;
    }

    int count = 1;
    double delay_sec = 1.0;

    BppToken tok = lex_peek(lex);
    if (tok.type != TOK_EOL && tok.type != TOK_EOF) {
        // Evaluate count expression
        BValue val_count = eval_expression(vm, lex, &err);
        if (err.code != 0) {
            return err;
        }
        if (val_count.type == VAL_STRING) {
            str_release(vm_get_str(vm), val_count.as.string);
            err.code = 13;
            err.message = "Type mismatch: BEEP count expects numeric argument";
            return err;
        }
        if (val_count.type != VAL_NUMBER) {
            err.code = 13;
            err.message = "Type mismatch: BEEP count expects numeric argument";
            return err;
        }

        if (val_count.as.number < 0.0) {
            err.code = 5;
            err.message = "Illegal function call: BEEP count cannot be negative";
            return err;
        }
        count = (int)val_count.as.number;

        // Check for optional comma and delay
        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA) {
            lex_next(lex); // Consume ','
            BValue val_delay = eval_expression(vm, lex, &err);
            if (err.code != 0) {
                return err;
            }
            if (val_delay.type == VAL_STRING) {
                str_release(vm_get_str(vm), val_delay.as.string);
                err.code = 13;
                err.message = "Type mismatch: BEEP delay expects numeric argument";
                return err;
            }
            if (val_delay.type != VAL_NUMBER) {
                err.code = 13;
                err.message = "Type mismatch: BEEP delay expects numeric argument";
                return err;
            }

            if (val_delay.as.number < 0.0) {
                err.code = 5;
                err.message = "Illegal function call: BEEP delay cannot be negative";
                return err;
            }
            delay_sec = val_delay.as.number;
        }
    }

    if (count <= 0) {
        return err; // 0 beeps: silent no-op
    }

    uint32_t delay_ms = (uint32_t)(delay_sec * 1000.0);

    platform_sound_stop();
    for (int i = 0; i < count; i++) {
        platform_sound_beep();
        if (i + 1 < count && delay_ms > 0) {
            platform_sleep_ms(delay_ms);
        }
    }
    platform_sound_stop();

    return err;
}



