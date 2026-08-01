/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file stmt_beep.c
 * @brief BEEP statement handler implementation.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements the BEEP statement. In standard builds, it triggers a synchronous
 *   800 Hz beep for 0.25 seconds if called with no arguments. If called with BEEP duration, pitch,
 *   it translates the pitch (semitones relative to middle C) to frequency and plays it for the duration.
 * - Why it exists: Provides legacy BASIC music capability and Sinclair Spectrum/CoCo audio feedback.
 * - Why it works this way: It parses optional arguments. If present, it computes the frequency using
 *   the standard semitone formula and calls vdev_play_sound_freq. Otherwise, it defaults to vdev_play_beep.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Default sound parameters.
 * - What cannot be changed: Security permissions checks.
 * - What to expect: Blocking behavior under standard builds to maintain timing characteristics.
 * - What to do if something breaks: Verify audio drivers are unmuted and initialized.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: VDevContext is initialized.
 * - Portability concerns: Requires math library (-lm) for pow().
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Register sound synthesis pipelines.
 */

#include "vm/vm.h"
#include "lexer/lexer.h"
#include "device/vdev.h"
#include "security/security.h"
#include "eval/eval.h"
#include <string.h>
#include <math.h>

BppError stmt_beep_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

#ifndef BPP_LITE_BUILD
    /* Security Check: requires virtual device access */
    if (security_check(SECOP_VDEV, 0) != 0) {
        err.code = 70; /* Permission Denied */
        err.message = "Permission denied: BEEP virtual device access blocked under sandbox settings";
        return err;
    }
#endif

    BppToken tok = lex_peek(lex);
    if (tok.type != TOK_EOL && tok.type != TOK_EOF && tok.type != TOK_DOUBLE_COLON && tok.start[0] != ':') {
        /* Sinclair BEEP duration, pitch */
        BValue dur_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;

        tok = lex_next(lex);
        if (tok.type != TOK_COMMA) {
            err.code = 2; err.message = "Expected ',' in BEEP";
            return err;
        }

        BValue pitch_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;

        if (dur_val.type == VAL_STRING || pitch_val.type == VAL_STRING) {
            err.code = 13; err.message = "Type mismatch: BEEP expects numeric arguments";
            return err;
        }

        double duration = dur_val.as.number;
        double pitch = pitch_val.as.number;

        /* Convert semitone pitch relative to middle C (261.63 Hz) */
        double freq = 261.63 * pow(2.0, pitch / 12.0);

        if (duration > 0.0 && freq > 20.0 && freq < 20000.0) {
#ifndef BPP_LITE_BUILD
            vdev_play_sound_freq(freq, duration);
#else
            (void)freq; (void)duration;
#endif
        }
    } else {
        /* Standard Beep */
        vdev_play_beep(vm_get_vdev(vm));
    }

    return err;
}
