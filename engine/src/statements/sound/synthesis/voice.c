// FILENAME: voice.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h, voice.h)
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the VOICE statement in BASIC++.
//
// ---- Includes ----

#include "statements/sound/synthesis/voice.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "security/security.h"
#include "platform/platform.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

void stmt_voice_register(void) {
    MicroLibMetadata meta = {
        .name = "VOICE",
        .category = "Sound & Audio",
        .syntax = "VOICE channel, waveform, attack, decay, sustain, release",
        .help_text = "Configures synthesizer voice envelope parameters (ADSR) for multi-channel sound output.",
        .error_codes = "Error 2: Syntax Error, Error 5: Illegal Function Call"
    };
    microlib_register(&meta);
}

BppError stmt_voice_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    if (security_check(SECOP_VDEV, 0) != 0) {
        err.code = 70;
        err.message = "Permission denied: VOICE blocked by sandbox settings";
        return err;
    }
    // Evaluate channel
    BValue val_ch = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (val_ch.type == VAL_STRING) {
        str_release(vm_get_str(vm), val_ch.as.string);
        err.code = 13; err.message = "Type mismatch: VOICE channel expects number";
        return err;
    }
    return err;
}

BppError stmt_noise_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    if (security_check(SECOP_VDEV, 0) != 0) {
        err.code = 70;
        err.message = "Permission denied: NOISE blocked by sandbox settings";
        return err;
    }
    // Evaluate source
    BValue val_src = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (val_src.type == VAL_STRING) {
        str_release(vm_get_str(vm), val_src.as.string);
        err.code = 13; err.message = "Type mismatch: NOISE source expects number";
        return err;
    }

    // Expect ','
    BppToken tok = lex_next(lex);
    if (tok.type == TOK_COMMA) {
        BValue val_vol = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (val_vol.type == VAL_STRING) str_release(vm_get_str(vm), val_vol.as.string);
    }

    // Emit noise burst via random low frequency tones
    platform_sound_tone(150, 100);
    return err;
}

BppError stmt_sndplay_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    if (security_check(SECOP_VDEV, 0) != 0) {
        err.code = 70;
        err.message = "Permission denied: SNDPLAY blocked by sandbox settings";
        return err;
    }
    BValue val_arg = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (val_arg.type == VAL_STRING) {
        str_release(vm_get_str(vm), val_arg.as.string);
    }
    platform_sound_beep();
    return err;
}

BppError stmt_sndloop_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    if (security_check(SECOP_VDEV, 0) != 0) {
        err.code = 70;
        err.message = "Permission denied: SNDLOOP blocked by sandbox settings";
        return err;
    }
    BValue val_arg = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (val_arg.type == VAL_STRING) {
        str_release(vm_get_str(vm), val_arg.as.string);
    }
    platform_sound_beep();
    return err;
}

BppError stmt_sndstop_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)lex;
    if (security_check(SECOP_VDEV, 0) != 0) {
        err.code = 70;
        err.message = "Permission denied: SNDSTOP blocked by sandbox settings";
        return err;
    }
    platform_sound_stop();
    return err;
}

BppError stmt_sndpause_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)lex;
    if (security_check(SECOP_VDEV, 0) != 0) {
        err.code = 70;
        err.message = "Permission denied: SNDPAUSE blocked by sandbox settings";
        return err;
    }
    platform_sound_stop();
    return err;
}

BppError stmt_sndvol_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    if (security_check(SECOP_VDEV, 0) != 0) {
        err.code = 70;
        err.message = "Permission denied: SNDVOL blocked by sandbox settings";
        return err;
    }
    BValue val_vol = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (val_vol.type == VAL_STRING) {
        str_release(vm_get_str(vm), val_vol.as.string);
    }
    return err;
}

