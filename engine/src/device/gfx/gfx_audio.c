// FILENAME: gfx_audio.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (ctype.h, ctype.c, hal.h, math.h, memops.h, memops.c)
// NEEDS: libcore (strops.h, strops.c)
// NEEDS: libengine (eval.h, eval.c, math.c)
// NEEDS: libkernel (gfx_internal.h, security.h, security.c)
// NEEDS: libplatform (platform.h)
// Implements virtual device and audio rendering logic for gfx_audio.
//
// ---- Includes ----

#include "device/gfx_internal.h"
#include "eval/eval.h"
#include "security/security.h"
#include "hal/hal.h"
#include "platform/platform.h"
#include "runtime/ctype/ctype.h"
#include "runtime/math/math.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

bool init_audio(void) {
    HalContext *hal = hal_get();
    if (hal && hal->audio.init) {
        return hal->audio.init();
    }
    return true;
}

void push_note(double frequency, double duration_seconds, bool background) {
    HalContext *hal = hal_get();
    if (frequency > 0.0 && duration_seconds > 0.0) {
        if (hal && hal->audio.tone) {
            hal->audio.tone((uint32_t)frequency, (uint32_t)(duration_seconds * 1000.0));
        }
    }
    if (!background && duration_seconds > 0.0) {
        platform_sleep_ms((uint32_t)(duration_seconds * 1000.0));
    }
}

void vdev_gfx_beep(VDevContext *ctx) {
    (void)ctx;
    HalContext *hal = hal_get();
    if (hal && hal->audio.beep) {
        hal->audio.beep();
    }
}

void vdev_play_sound_freq(double freq, double duration_seconds) {
    push_note(freq, duration_seconds, true);
}

int vdev_music_note_count(void) { return 0; }
int vdev_music_queue_length(void) { return 0; }
void vdev_music_clear(void) {
    HalContext *hal = hal_get();
    if (hal && hal->audio.stop) {
        hal->audio.stop();
    }
}

// parses and schedules Music Macro Language (MML) strings
void play_mml(VMContext *vm, const char *mml) {
    (void)vm;
    if (!mml) return;

    int tempo = 120;
    int octave = 4;
    int length = 4;
    bool background = true;

    const char *p = mml;
    while (*p) {
        while (*p && runtime_isspace((unsigned char)*p)) p++;
        if (!*p) break;

        char cmd = (char)runtime_toupper((unsigned char)*p);
        p++;

        if (cmd == 'T') {
            int val = 0;
            while (*p && runtime_isdigit((unsigned char)*p)) {
                val = val * 10 + (*p - '0');
                p++;
            }
            if (val >= 32 && val <= 255) tempo = val;
        } else if (cmd == 'O') {
            int val = 0;
            while (*p && runtime_isdigit((unsigned char)*p)) {
                val = val * 10 + (*p - '0');
                p++;
            }
            if (val >= 0 && val <= 6) octave = val;
        } else if (cmd == 'L') {
            int val = 0;
            while (*p && runtime_isdigit((unsigned char)*p)) {
                val = val * 10 + (*p - '0');
                p++;
            }
            if (val >= 1 && val <= 64) length = val;
        } else if (cmd == 'M') {
            if (*p) {
                char mode = (char)runtime_toupper((unsigned char)*p);
                p++;
                if (mode == 'B') background = true;
                else if (mode == 'F') background = false;
            }
        } else if (cmd == '<') {
            if (octave > 0) octave--;
        } else if (cmd == '>') {
            if (octave < 6) octave++;
        } else if (cmd >= 'A' && cmd <= 'G') {
            int note_offset = 0;
            switch (cmd) {
                case 'C': note_offset = 0; break;
                case 'D': note_offset = 2; break;
                case 'E': note_offset = 4; break;
                case 'F': note_offset = 5; break;
                case 'G': note_offset = 7; break;
                case 'A': note_offset = 9; break;
                case 'B': note_offset = 11; break;
            }
            if (*p == '+' || *p == '#') { note_offset++; p++; }
            else if (*p == '-') { note_offset--; p++; }

            int note_len = length;
            int len_val = 0;
            while (*p && runtime_isdigit((unsigned char)*p)) {
                len_val = len_val * 10 + (*p - '0');
                p++;
            }
            if (len_val >= 1 && len_val <= 64) note_len = len_val;

            double dur_mult = 1.0;
            if (*p == '.') { dur_mult = 1.5; p++; }

            int midi = (octave + 1) * 12 + note_offset;
            double freq = 440.0 * runtime_pow(2.0, (midi - 69) / 12.0);
            double dur = (240.0 / ((double)tempo * (double)note_len)) * dur_mult;

            push_note(freq, dur, background);
        } else if (cmd == 'P') {
            int note_len = length;
            int len_val = 0;
            while (*p && runtime_isdigit((unsigned char)*p)) {
                len_val = len_val * 10 + (*p - '0');
                p++;
            }
            if (len_val >= 1 && len_val <= 64) note_len = len_val;

            double dur_mult = 1.0;
            if (*p == '.') { dur_mult = 1.5; p++; }

            double dur = (240.0 / ((double)tempo * (double)note_len)) * dur_mult;
            push_note(0.0, dur, background);
        } else if (cmd == 'N') {
            int note_val = 0;
            while (*p && runtime_isdigit((unsigned char)*p)) {
                note_val = note_val * 10 + (*p - '0');
                p++;
            }
            if (note_val >= 0 && note_val <= 84) {
                if (note_val == 0) {
                    double dur = 240.0 / ((double)tempo * (double)length);
                    push_note(0.0, dur, background);
                } else {
                    int midi = 11 + note_val;
                    double freq = 440.0 * runtime_pow(2.0, (midi - 69) / 12.0);
                    double dur = 240.0 / ((double)tempo * (double)length);
                    push_note(freq, dur, background);
                }
            }
        }
    }
}

int vdev_sound_open(const char *filename) { (void)filename; return -1; }
void vdev_sound_play(int handle) { (void)handle; }
void vdev_sound_loop(int handle) { (void)handle; }
void vdev_sound_stop(int handle) { (void)handle; vdev_music_clear(); }
void vdev_sound_pause(int handle) { (void)handle; }
void vdev_sound_volume(int handle, double vol) {
    (void)handle;
    HalContext *hal = hal_get();
    if (hal && hal->audio.set_volume) {
        hal->audio.set_volume((uint8_t)(vol * 100.0));
    }
}
double vdev_sound_length(int handle) { (void)handle; return 0.0; }
double vdev_sound_position(int handle) { (void)handle; return 0.0; }
void vdev_sound_noise(int type, double dur_seconds) { (void)type; (void)dur_seconds; }
void vdev_sound_free_all(void) {}

// evaluates and dispatches SOUND statements
BppError vdev_legacy_stmt_sound_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

#ifndef BASIC_LITE_BUILD
    if (security_check(SECOP_VDEV, 0) != 0) {
        err.code = 70;
        err.message = "Permission denied: SOUND virtual device access blocked under sandbox settings";
        return err;
    }
#endif

    BValue freq_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    BppToken tok = lex_next(lex);
    if (tok.type != TOK_COMMA) {
        err.code = 2; err.message = "Expected ',' in SOUND";
        return err;
    }

    BValue dur_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (freq_val.type == VAL_STRING || dur_val.type == VAL_STRING) {
        err.code = 13; err.message = "Type mismatch: SOUND expects numeric arguments";
        return err;
    }

    double freq = freq_val.as.number;
    double dur_ticks = dur_val.as.number;

    if (freq < 0.0 || freq > 32767.0) {
        err.code = 5; err.message = "Illegal function call: SOUND frequency must be 0 to 32767 Hz";
        return err;
    }
    if (dur_ticks < 0.0 || dur_ticks > 65535.0) {
        err.code = 5; err.message = "Illegal function call: SOUND duration must be 0 to 65535 ticks";
        return err;
    }

    double duration_sec = dur_ticks / 18.2;
    push_note(freq, duration_sec, true);

    return err;
}

// evaluates and dispatches PLAY statement music strings
BppError vdev_legacy_stmt_play_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

#ifndef BASIC_LITE_BUILD
    if (security_check(SECOP_VDEV, 0) != 0) {
        err.code = 70;
        err.message = "Permission denied: PLAY virtual device access blocked under sandbox settings";
        return err;
    }
#endif

    BppToken peek = lex_peek(lex);
    if (peek.type == TOK_KEYWORD && (peek.as.keyword == KW_ON || peek.as.keyword == KW_OFF || peek.as.keyword == KW_STOP)) {
        lex_next(lex);
        if (peek.as.keyword == KW_ON) {
            vm_set_play_state(vm, 1);
        } else if (peek.as.keyword == KW_OFF) {
            vm_set_play_state(vm, 0);
        } else {
            vm_set_play_state(vm, 2);
        }
        return err;
    }

    BValue cmd_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (cmd_val.type != VAL_STRING) {
        err.code = 13; err.message = "Type mismatch: PLAY expects command string";
        return err;
    }

    const char *mml_str = str_data(cmd_val.as.string);
    if (!mml_str || mml_str[0] == '\0') {
        vdev_music_clear();
    } else {
        play_mml(vm, mml_str);
    }

    str_release(vm_get_str(vm), cmd_val.as.string);
    return err;
}
