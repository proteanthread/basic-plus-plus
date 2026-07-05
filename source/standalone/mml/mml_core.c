/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: mml_core.c
 * Subsystem: Sound Music Macro Language Composer
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Parses MML sound strings into pitch/frequency arrays.
 *
 * 2. WHAT TO EXPECT:
 *    Translates macros (notes, tempo, octave) to sound events.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Macro syntax keys, note limits.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    MML syntax scanner.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If notes skip or sound wrong, verify macro bounds.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE MML MUSIC PARSER
 * File: mml_core.c
 * ===================================================================== */

#include "mml_core.h"

int mml_core_play(const char *mml_str, int length,
                  MmlSoundCallback sound_cb,
                  MmlSleepCallback sleep_cb,
                  void *user_data)
{
    static const int note_freq4[] = {
        262, 277, 294, 311, 330, 349,
        370, 392, 415, 440, 466, 494
    };
    static const int note_semi[] = {
        9, 11, 0, 2, 4, 5, 7
    }; /* A B C D E F G */

    int octave = 4;
    int note_len = 4;
    int tempo = 120;
    int si;

    for (si = 0; si < length; si++) {
        char ch = mml_str[si];
        if (ch >= 'a' && ch <= 'z') {
            ch = (char)(ch - 32);
        }

        if (ch >= 'A' && ch <= 'G') {
            int semi = note_semi[ch - 'A'];
            int this_len = note_len;
            int dotted = 0;
            int freq, dur;

            /* Check sharp/flat */
            if (si + 1 < length && (mml_str[si + 1] == '#' || mml_str[si + 1] == '+')) {
                semi++;
                if (semi > 11) semi = 0;
                si++;
            } else if (si + 1 < length && mml_str[si + 1] == '-') {
                semi--;
                if (semi < 0) semi = 11;
                si++;
            }

            /* Check length suffix */
            if (si + 1 < length && mml_str[si + 1] >= '1' && mml_str[si + 1] <= '9') {
                int num = 0;
                si++;
                while (si < length && mml_str[si] >= '0' && mml_str[si] <= '9') {
                    num = num * 10 + (mml_str[si] - '0');
                    si++;
                }
                si--;
                if (num >= 1 && num <= 64) {
                    this_len = num;
                }
            }

            /* Check dot */
            if (si + 1 < length && mml_str[si + 1] == '.') {
                dotted = 1;
                si++;
            }

            /* Calculate frequency */
            freq = note_freq4[semi];
            if (octave < 4) {
                int sh;
                for (sh = 0; sh < 4 - octave; sh++) {
                    freq /= 2;
                }
            } else if (octave > 4) {
                int sh;
                for (sh = 0; sh < octave - 4; sh++) {
                    freq *= 2;
                }
            }

            /* Calculate duration */
            dur = 240000 / (tempo * this_len);
            if (dotted) {
                dur = dur + dur / 2;
            }

            if (sound_cb) {
                sound_cb(user_data, freq, dur);
            }

        } else if (ch == 'O') {
            if (si + 1 < length && mml_str[si + 1] >= '0' && mml_str[si + 1] <= '6') {
                octave = mml_str[si + 1] - '0';
                si++;
            }
        } else if (ch == '>') {
            if (octave < 6) octave++;
        } else if (ch == '<') {
            if (octave > 0) octave--;
        } else if (ch == 'L') {
            int num = 0;
            si++;
            while (si < length && mml_str[si] >= '0' && mml_str[si] <= '9') {
                num = num * 10 + (mml_str[si] - '0');
                si++;
            }
            si--;
            if (num >= 1 && num <= 64) {
                note_len = num;
            }
        } else if (ch == 'T') {
            int num = 0;
            si++;
            while (si < length && mml_str[si] >= '0' && mml_str[si] <= '9') {
                num = num * 10 + (mml_str[si] - '0');
                si++;
            }
            si--;
            if (num >= 32 && num <= 255) {
                tempo = num;
            }
        } else if (ch == 'P') {
            int plen = note_len;
            int dur;
            if (si + 1 < length && mml_str[si + 1] >= '1' && mml_str[si + 1] <= '9') {
                int num = 0;
                si++;
                while (si < length && mml_str[si] >= '0' && mml_str[si] <= '9') {
                    num = num * 10 + (mml_str[si] - '0');
                    si++;
                }
                si--;
                if (num >= 1 && num <= 64) {
                    plen = num;
                }
            }
            dur = 240000 / (tempo * plen);
            if (sleep_cb) {
                sleep_cb(user_data, dur);
            }
        } else if (ch == 'N') {
            int num = 0;
            int freq, oct, semi, dur;
            si++;
            while (si < length && mml_str[si] >= '0' && mml_str[si] <= '9') {
                num = num * 10 + (mml_str[si] - '0');
                si++;
            }
            si--;
            if (num == 0) {
                dur = 240000 / (tempo * note_len);
                if (sleep_cb) {
                    sleep_cb(user_data, dur);
                }
            } else if (num <= 84) {
                num--;
                oct = num / 12;
                semi = num % 12;
                freq = note_freq4[semi];
                if (oct < 4) {
                    int sh;
                    for (sh = 0; sh < 4 - oct; sh++) {
                        freq /= 2;
                    }
                } else if (oct > 4) {
                    int sh;
                    for (sh = 0; sh < oct - 4; sh++) {
                        freq *= 2;
                    }
                }
                dur = 240000 / (tempo * note_len);
                if (sound_cb) {
                    sound_cb(user_data, freq, dur);
                }
            }
        }
    }
    return 0;
}
