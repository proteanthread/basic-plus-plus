// FILENAME: play.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (ctype.h, ctype.c, micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libcore (string.h, strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, play.h, string.c, vm.h)
// NEEDS: libkernel (errors.h, security.h, security.c, vdev.h, vdev.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the PLAY statement in BASIC++.
//
// ---- Includes ----

#include "statements/sound/synthesis/play.h"
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
#include <ctype.h>

void stmt_play_register(void) {
    static const MicroLibMetadata meta = {
        .name = "PLAY",
        .category = "Sound & Audio",
        .syntax = "PLAY command_string",
        .help_text = "Plays musical notes using MML (Music Macro Language) command string.",
        .error_codes = "Error 2: Syntax Error, Error 5: Illegal Function Call"
    };
    microlib_register(&meta);
}

typedef struct {
    int octave;        // 0..6, default 4
    int length;        // 1..64, default 4
    int tempo;         // 32..255, default 120 (BPM)
    int volume;        // 0..15, default 15
    int music_mode;    // 0=Normal (7/8 sound), 1=Legato (8/8 sound), 2=Staccato (3/4 sound)
    bool background;   // false=MF (foreground), true=MB (background)
} MMLState;

static uint32_t mml_note_freq(int note_num) {
    if (note_num < 0 || note_num > 84) return 0;
    static const uint16_t freq_table[85] = {
        16,   17,   18,   19,   21,   22,   23,   25,   26,   28,   29,   31,   // Octave 0
        33,   35,   37,   39,   41,   44,   46,   49,   52,   55,   58,   62,   // Octave 1
        65,   69,   73,   78,   82,   87,   93,   98,   104,  110,  117,  123,  // Octave 2
        131,  139,  147,  156,  165,  175,  185,  196,  208,  220,  233,  247,  // Octave 3
        262,  277,  294,  311,  330,  349,  370,  392,  415,  440,  466,  494,  // Octave 4
        523,  554,  587,  622,  659,  698,  740,  784,  831,  880,  932,  988,  // Octave 5
        1047, 1109, 1175, 1245, 1319, 1397, 1480, 1568, 1661, 1760, 1865, 1976, // Octave 6
        2093                                                                  // C7 (84)
    };
    return (uint32_t)freq_table[note_num];
}

static void mml_play_string(const char *str) {
    if (!str) return;
    MMLState state = {
        .octave = 4,
        .length = 4,
        .tempo = 120,
        .volume = 15,
        .music_mode = 0,
        .background = false
    };

    const char *p = str;
    while (*p != '\0') {
        char c = *p;
        if (c == ' ' || c == '\t' || c == ';' || c == ',') {
            p++;
            continue;
        }

        char uc = (char)toupper((unsigned char)c);

        if (uc == 'O') {
            p++;
            if (*p >= '0' && *p <= '6') {
                state.octave = *p - '0';
                p++;
            }
            continue;
        }
        if (uc == '>') {
            if (state.octave < 6) state.octave++;
            p++;
            continue;
        }
        if (uc == '<') {
            if (state.octave > 0) state.octave--;
            p++;
            continue;
        }
        if (uc == 'L') {
            p++;
            int val = 0;
            while (*p >= '0' && *p <= '9') {
                val = val * 10 + (*p - '0');
                p++;
            }
            if (val >= 1 && val <= 64) {
                state.length = val;
            }
            continue;
        }
        if (uc == 'T') {
            p++;
            int val = 0;
            while (*p >= '0' && *p <= '9') {
                val = val * 10 + (*p - '0');
                p++;
            }
            if (val >= 32 && val <= 255) {
                state.tempo = val;
            }
            continue;
        }
        if (uc == 'V') {
            p++;
            int val = 0;
            while (*p >= '0' && *p <= '9') {
                val = val * 10 + (*p - '0');
                p++;
            }
            if (val >= 0 && val <= 15) {
                state.volume = val;
            }
            continue;
        }
        if (uc == 'M') {
            p++;
            char m_c = (char)toupper((unsigned char)*p);
            if (m_c == 'N') { state.music_mode = 0; p++; }
            else if (m_c == 'L') { state.music_mode = 1; p++; }
            else if (m_c == 'S') { state.music_mode = 2; p++; }
            else if (m_c == 'B') { state.background = true; p++; }
            else if (m_c == 'F') { state.background = false; p++; }
            continue;
        }
        if (uc == 'N') {
            p++;
            int note_num = 0;
            while (*p >= '0' && *p <= '9') {
                note_num = note_num * 10 + (*p - '0');
                p++;
            }
            if (note_num == 0) {
                uint32_t duration_ms = (uint32_t)((240000.0 / state.tempo) / state.length);
                platform_sound_tone(0, duration_ms);
            } else if (note_num >= 1 && note_num <= 84) {
                uint32_t freq = mml_note_freq(note_num - 1);
                uint32_t full_ms = (uint32_t)((240000.0 / state.tempo) / state.length);
                uint32_t play_ms = full_ms;
                uint32_t rest_ms = 0;

                if (state.music_mode == 0) {
                    play_ms = (full_ms * 7) / 8;
                    rest_ms = full_ms - play_ms;
                } else if (state.music_mode == 2) {
                    play_ms = (full_ms * 3) / 4;
                    rest_ms = full_ms - play_ms;
                }

                platform_sound_tone(freq, play_ms);
                if (rest_ms > 0) platform_sound_tone(0, rest_ms);
            }
            continue;
        }
        if (uc == 'P' || uc == 'R') {
            p++;
            int rest_len = state.length;
            if (*p >= '0' && *p <= '9') {
                rest_len = 0;
                while (*p >= '0' && *p <= '9') {
                    rest_len = rest_len * 10 + (*p - '0');
                    p++;
                }
            }
            if (rest_len < 1) rest_len = 1;
            if (rest_len > 64) rest_len = 64;

            uint32_t duration_ms = (uint32_t)((240000.0 / state.tempo) / rest_len);
            if (*p == '.') {
                duration_ms = (duration_ms * 3) / 2;
                p++;
            }
            platform_sound_tone(0, duration_ms);
            continue;
        }
        if (uc >= 'A' && uc <= 'G') {
            p++;
            int base_idx = 0;
            switch (uc) {
                case 'C': base_idx = 0; break;
                case 'D': base_idx = 2; break;
                case 'E': base_idx = 4; break;
                case 'F': base_idx = 5; break;
                case 'G': base_idx = 7; break;
                case 'A': base_idx = 9; break;
                case 'B': base_idx = 11; break;
            }

            if (*p == '#' || *p == '+') {
                base_idx++;
                p++;
            } else if (*p == '-') {
                base_idx--;
                p++;
            }

            int note_len = state.length;
            if (*p >= '0' && *p <= '9') {
                note_len = 0;
                while (*p >= '0' && *p <= '9') {
                    note_len = note_len * 10 + (*p - '0');
                    p++;
                }
            }
            if (note_len < 1) note_len = 1;
            if (note_len > 64) note_len = 64;

            double dot_factor = 1.0;
            double add_factor = 0.5;
            while (*p == '.') {
                dot_factor += add_factor;
                add_factor /= 2.0;
                p++;
            }

            int note_num = state.octave * 12 + base_idx;
            if (note_num >= 0 && note_num <= 84) {
                uint32_t freq = mml_note_freq(note_num);
                uint32_t full_ms = (uint32_t)(((240000.0 / state.tempo) / note_len) * dot_factor);
                uint32_t play_ms = full_ms;
                uint32_t rest_ms = 0;

                if (state.music_mode == 0) {
                    play_ms = (full_ms * 7) / 8;
                    rest_ms = full_ms - play_ms;
                } else if (state.music_mode == 2) {
                    play_ms = (full_ms * 3) / 4;
                    rest_ms = full_ms - play_ms;
                }

                platform_sound_tone(freq, play_ms);
                if (rest_ms > 0) platform_sound_tone(0, rest_ms);
            }
            continue;
        }

        p++;
    }
}

BppError stmt_play_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (security_check(SECOP_VDEV, 0) != 0) {
        err.code = 70;
        err.message = "Permission denied: PLAY blocked by sandbox settings";
        return err;
    }

    BValue val_cmd = eval_expression(vm, lex, &err);
    if (err.code != 0) {
        return err;
    }
    if (val_cmd.type != VAL_STRING || !val_cmd.as.string) {
        if (val_cmd.type == VAL_STRING && val_cmd.as.string) {
            str_release(vm_get_str(vm), val_cmd.as.string);
        }
        err.code = 13;
        err.message = "Type mismatch: PLAY expects MML command string argument";
        return err;
    }

    const char *data_ptr = str_data(val_cmd.as.string);
    mml_play_string(data_ptr);
    str_release(vm_get_str(vm), val_cmd.as.string);

    return err;
}


