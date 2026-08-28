// FILENAME: sound.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, sound.h, string.c, vm.h)
// NEEDS: libkernel (errors.h, security.h, security.c, vdev.h, vdev.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the SOUND statement in BASIC++.
//
// ---- Includes ----

#include "statements/sound/synthesis/sound.h"
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

void stmt_sound_register(void) {
    static const MicroLibMetadata meta = {
        .name = "SOUND",
        .category = "Sound & Audio",
        .syntax = "SOUND frequency, duration",
        .help_text = "Generates a tone of specified frequency in Hertz for specified duration in clock ticks.",
        .error_codes = "Error 2: Syntax Error, Error 5: Illegal Function Call"
    };
    microlib_register(&meta);
}

BppError stmt_sound_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (security_check(SECOP_VDEV, 0) != 0) {
        err.code = 70;
        err.message = "Permission denied: SOUND blocked by sandbox settings";
        return err;
    }

    // Evaluate frequency
    BValue val_freq = eval_expression(vm, lex, &err);
    if (err.code != 0) {
        return err;
    }
    if (val_freq.type == VAL_STRING) {
        str_release(vm_get_str(vm), val_freq.as.string);
        err.code = 13;
        err.message = "Type mismatch: SOUND frequency expects numeric argument";
        return err;
    }
    if (val_freq.type != VAL_NUMBER) {
        err.code = 13;
        err.message = "Type mismatch: SOUND frequency expects numeric argument";
        return err;
    }

    double freq_num = val_freq.as.number;
    if (freq_num < 0.0 || freq_num > 32767.0 || (freq_num > 0.0 && freq_num < 37.0)) {
        err.code = 5;
        err.message = "Illegal function call: SOUND frequency must be 0 or 37 to 32767 Hz";
        return err;
    }

    // Expect ',' separator
    BppToken tok = lex_next(lex);
    if (tok.type != TOK_COMMA) {
        err.code = 2;
        err.message = "Syntax error: Expected ',' in SOUND";
        return err;
    }

    // Evaluate duration
    BValue val_dur = eval_expression(vm, lex, &err);
    if (err.code != 0) {
        return err;
    }
    if (val_dur.type == VAL_STRING) {
        str_release(vm_get_str(vm), val_dur.as.string);
        err.code = 13;
        err.message = "Type mismatch: SOUND duration expects numeric argument";
        return err;
    }
    if (val_dur.type != VAL_NUMBER) {
        err.code = 13;
        err.message = "Type mismatch: SOUND duration expects numeric argument";
        return err;
    }

    double dur_num = val_dur.as.number;
    if (dur_num < 0.0 || dur_num > 65535.0) {
        err.code = 5;
        err.message = "Illegal function call: SOUND duration must be 0 to 65535 ticks";
        return err;
    }

    // Duration in ticks (18.2 ticks/sec) -> ms
    uint32_t dur_ms = (uint32_t)(dur_num * 1000.0 / 18.2);
    uint32_t freq_hz = (uint32_t)freq_num;

    // Optional volume (Tandy 1000 / PCjr: 0..15)
    tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex); // Consume ','
        BValue val_vol = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (val_vol.type == VAL_STRING) {
            str_release(vm_get_str(vm), val_vol.as.string);
        }

        // Optional voice channel (Tandy 1000 / PCjr: 0..2)
        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA) {
            lex_next(lex); // Consume ','
            BValue val_voice = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (val_voice.type == VAL_STRING) {
                str_release(vm_get_str(vm), val_voice.as.string);
            }
        }
    }

    platform_sound_tone(freq_hz, dur_ms);

    return err;
}



